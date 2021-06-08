//
//  Database.cpp
//  RGAssignment2
//
//  Created by rick gessner on 2/27/21.
//

#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <map>
#include <memory>
#include <vector>
#include "BasicTypes.hpp"
#include "Storage.hpp"
#include "Database.hpp"
#include "Config.hpp"
#include "BlockIO.hpp"
#include "Row.hpp"

namespace ECE141 {
  
  Database::Database(const std::string aName, CreateDB)
    : name(aName), storage(stream), changed(true)  {
      std::string thePath = Config::getDBPath(name);
      stream.clear(); // Clear Flag, then create file...
      stream.open(thePath.c_str(), std::fstream::binary | std::fstream::in | std::fstream::out | std::fstream::trunc);
      stream.close();
      stream.open(thePath.c_str(), std::fstream::binary | std::fstream::binary | std::fstream::in | std::fstream::out);
      
      std::stringstream ss;
      //encode and create a new block
      if (encode(ss) == Errors::noError) {
          StorageInfo info(0, ss.str().size(), kNewBlock, BlockType::meta_block);
          storage.save(ss, info);
      }
  }

  Database::Database(const std::string aName, OpenDB)
    : name(aName), changed(false), storage(stream) {
      
      std::string thePath = Config::getDBPath(name);
      stream.open (thePath.c_str(), std::fstream::binary | std::fstream::in | std::fstream::out);
      
      //read and decode the meta block
      std::stringstream ss;
      if (storage.load(ss, 0) == Errors::noError) {
          decode(ss);
      }

      //load entities into memory
      for (auto& cur : tables) {
          std::stringstream ss2;
          storage.load(ss2, cur.second);
          Entity theEntity(cur.first, {});
          theEntity.decode(ss2);
          entities.push_back(theEntity);
      }

      //load indexes into memory
      for (auto& cur : indexBlockNums) {
          std::stringstream ss2;
          storage.load(ss2, cur);
          Index theIndex(storage, cur);
          theIndex.decode(ss2);
          indexes.push_back(theIndex);
      }

  }

  Database::~Database() {
      if(changed) {          
          //update meta information
          std::stringstream ss;
          this->encode(ss);

          StorageInfo theMetaInfo(0, ss.str().size(), 0, BlockType::meta_block);
          storage.save(ss, theMetaInfo);
          
          //update entities
          for (auto& entity : entities) {
              std::stringstream ss2;
              entity.encode(ss2);
              StorageInfo theInfo(entity.hashName(), ss2.str().size(), tables[entity.getName()], BlockType::entity_block);
              storage.save(ss2, theInfo);
          }

          //update indexes
          for (auto& cur : indexes) {
              if (cur.isChanged()) {
                  std::stringstream ss2;
                  cur.encode(ss2);
                  StorageInfo theInfo = cur.getStorageInfo(ss2.str().size());
                  storage.save(ss2, theInfo);
              }
          }
      }
      //close the stream
      stream.close();
  }

  Entity* Database::getEntity(std::string aName) {
      //iterate through table list and return the entity
      for (auto& entity : entities) {
          if (entity.getName() == aName) {
              return &entity;
          }
      }

      //table not found
      return nullptr;
  }

  std::vector<std::string> Database::getTableNames() {
      std::vector<std::string> res;

      for (auto& table : tables)
          res.push_back(table.first);

      return res;
  }

  IndexPairs Database::getIndex(std::string aTableName, std::vector<std::string> aFields) {
      IndexPairs res;
      for (auto& index : indexes) {
          if (index.getTableName() == aTableName) {
              for (auto& field : aFields) {
                  if (index.getFieldName() == field) {
                      IndexPairs thePairs = index.getIndexPairs();
                      res.insert(res.end(), thePairs.begin(), thePairs.end());
                  }
              }
          }
      }
      return res;
  }

  IndexPairs Database::getAllIndexes() {
      IndexPairs res;
      for (auto& index : indexes) {
          res.push_back({ index.getTableName(), index.getFieldName() });
      }
      return res;
  }

  void Database::deleteIndexes(KeyValues& aKeyValue) {
      for (auto& index : indexes) {
          std::string theField = index.getFieldName();
          if (index.getType() == IndexType::intKey) {
              uint32_t theKey = std::get<int>(aKeyValue[theField]);
              index.erase(theKey);
          }
          else {
              std::string theKey = std::get<std::string>(aKeyValue[theField]);
              index.erase(theKey);
          }
      }
  }

  void Database::deleteAllIndexes(std::string aTableName) {
      std::vector<Index> newIndexes;

      for (auto& index : indexes) {
          if (index.getTableName() == aTableName) {
              storage.markBlockAsFree(index.getBlockNum());
              indexBlockNums.erase(index.getBlockNum());
          }
          else {
              newIndexes.push_back(index);
          }
      }

      indexes = newIndexes;
  }
  
  void Database::insertIndexes(std::vector<Index*> anIndexes, KeyValues& aKeyValue, uint32_t blockNum) {
      for (auto* index : anIndexes) {
          std::string theField = index->getFieldName();
          if (index->getType() == IndexType::intKey) {
              uint32_t theKey = std::get<int>(aKeyValue[theField]);
              Index::ValueProxy theProxy = (*index)[theKey];
              theProxy = blockNum;
          }
          else {
              std::string theKey = std::get<std::string> (aKeyValue[theField]);
              Index::ValueProxy theProxy = (*index)[theKey];
              theProxy = blockNum;
          }
      }
  }
   
  StatusResult Database::addTable(std::string aName, const std::vector<Attribute>& anAttributes) {
      if (tables.count(aName))
          return StatusResult{ Errors::tableExists };
      
      Entity theEntity(aName, anAttributes);
      
      //update entity information
      auto blockNum = storage.getNextFreeBlockNum();
      entities.push_back(theEntity);
      tables[aName] = blockNum;
      
      //write into database
      std::stringstream ss;
      theEntity.encode(ss);
      StorageInfo theInfo(theEntity.hashName(), ss.str().size(), kNewBlock, BlockType::entity_block);
      storage.save(ss, theInfo);

      //set up index with primary key
      Attribute* primaryAtt = theEntity.getPrimaryKey();
      uint32_t indexBlockNum = storage.getNextFreeBlockNum();
      IndexType indexType;
      if (primaryAtt->getType() == DataTypes::int_type)
          indexType = IndexType::intKey;
      else
          indexType = IndexType::strKey;
      
      //write to file
      Index theIndex(storage, indexBlockNum, aName, primaryAtt->getName(), indexType);
      std::stringstream ss2;
      theIndex.encode(ss2);
      StorageInfo indexInfo = theIndex.getStorageInfo(ss2.str().size());
      indexInfo.start = kNewBlock;
      storage.save(ss2, indexInfo);

      indexes.push_back(theIndex);
      indexBlockNums.insert(indexBlockNum);
      
      //status changed, update data when closing database
      changed = true;
      return StatusResult{ Errors::noError };
  }

  StatusResult Database::dropTable(std::string aName) {
      if (!tables.count(aName))
          return StatusResult{ Errors::unknownTable };

      uint32_t count = 0;
      uint32_t blockNum = tables[aName];
      //remove from table map
      tables.erase(aName);

      //erase from table list
      for (size_t i = 0; i < entities.size(); ++i) {
          if (entities[i].getName() == aName) {
              //set up a query with no filter
              std::shared_ptr<Query> theQuery = std::make_unique<Query>();
              theQuery->setFrom(&entities[i]);

              //delete all rows
              StatusResult result = deleteRows(theQuery);
              
              //delete all the associated indexes
              deleteAllIndexes(entities[i].getName());

              if (!result)
                  return result;
              else
                  count += result.value;

              //remove from entity list
              entities.erase(entities.begin() + i);

              break;
          }
      }

      storage.markBlockAsFree(blockNum);
      changed = true;
      return StatusResult{ Errors::noError, count };
  }

  StatusResult Database::alterRow(Attribute& anAtt, Keywords aMode, std::string aTableName, std::string aPrimaryKey) {
      for (auto& index : indexes) {
          if (index.getTableName() == aTableName && index.getFieldName() == aPrimaryKey) {
              index.each([&](const Block& theBlock, uint32_t blockIndex)->bool {
                  //read and decode row data
                  std::stringstream ss;
                  ss.write(theBlock.payload, theBlock.header.size);
                  Row row;
                  row.decode(ss);

                  //update and encode
                  std::stringstream news; //a new stream
                  if (aMode == Keywords::add_kw)
                      row.addData(anAtt.getName(), std::string(""));
                  else
                      row.dropData(anAtt.getName());

                  row.encode(news);

                  //prepare for a new block
                  Block newBlock = theBlock;
                  newBlock.header.size = news.str().size();
                  news.read(newBlock.payload, newBlock.header.size);

                  //overwrite data
                  storage.writeBlock(blockIndex, newBlock);
                  return true;
                  }
              );
              break;
          }
      }
      return StatusResult{ Errors::noError };
  }

  StatusResult Database::alterTable(std::string aTableName, Keywords aMode, Attribute& anAtt) {
      if (!tables.count(aTableName))
          return StatusResult{ Errors::unknownTable };

      StatusResult result{ Errors::noError };
      Entity* theEntity = getEntity(aTableName);
      std::string primaryKey = theEntity->getPrimaryKey()->getName();

      if (aMode == Keywords::add_kw)
          theEntity->addAttribute(anAtt);
      else
          theEntity->dropAttribute(anAtt);

      result = alterRow(anAtt, aMode, aTableName, primaryKey);

      changed = true;
      return result;
  }
  
  //accept a string, return a bool
  static bool stob(std::string aStr) {
      if (aStr == "0")
          return false;
      else
          return true;
  }

  static void buildKeyValueList(std::vector<KeyValues>& keyValueList,
      Entity* anEntity,
      const std::vector<std::string>& anAttNames,
      const std::vector<std::vector<std::string>>& aValues) {

      int n = aValues[0].size(); //number of input attributes
      int m = aValues.size(); //number of input rows
      
      //iterate through given attributes and build key/value(row) list
      for (int i = 0; i < n; ++i) { //i is the index of attribute(column)
          Attribute* theAtt = anEntity->getAttribute(anAttNames[i]);
         
          switch (theAtt->getType()) {
          case(DataTypes::bool_type):
              for (int j = 0; j < m; ++j) //j is the index of data(row)
                  keyValueList[j][theAtt->getName()] = stob(aValues[j][i]);
              break;

          case(DataTypes::datetime_type):
              for (int j = 0; j < m; ++j) //j is the index of data(row)
                  keyValueList[j][theAtt->getName()] = aValues[j][i];
              break;

          case(DataTypes::float_type):
              for (int j = 0; j < m; ++j) //j is the index of data(row)
                  keyValueList[j][theAtt->getName()] = std::stod(aValues[j][i]);
              break;

          case(DataTypes::int_type):
              for (int j = 0; j < m; ++j) //j is the index of data(row)
                  keyValueList[j][theAtt->getName()] = std::stoi(aValues[j][i]);
              break;

          case(DataTypes::varchar_type):
              for (int j = 0; j < m; ++j) //j is the index of data(row)
                  keyValueList[j][theAtt->getName()] = aValues[j][i].substr(0, theAtt->getLength());
              break;

          default:
              break;
          }
      }
  }

  StatusResult Database::insertRows(std::string aTableName, const std::vector<std::string>& anAttNames, 
      const std::vector<std::vector<std::string>>& aValues) {
      uint32_t affectedRows = 0;

      //find the table
      Entity* theTable = nullptr;
      for (auto& entity : entities) {
          if (entity.getName() == aTableName) {
              theTable = &entity;
              break;
          }
      }

      //if the table is not found
      if (!theTable)
          return StatusResult{ Errors::unknownTable, affectedRows };

      //build key value list for creating rows
      std::vector<KeyValues> keyValueList(aValues.size());
      buildKeyValueList(keyValueList, theTable, anAttNames, aValues);

      //find all corresponding indexes
      std::vector<Index*> tableIndexes;
      for (auto& index : indexes) {
          if (index.getTableName() == theTable->getName())
              tableIndexes.push_back(&index);
      }
      
      for (auto& keyValue : keyValueList) {
          ++affectedRows;
          auto blockNum = storage.getNextFreeBlockNum();
          auto id = theTable->getIncrement();
          keyValue["id"] = int(id);

          Row theRow(keyValue, blockNum);
          std::stringstream ss;
          theRow.encode(ss);
          size_t size = ss.str().size();

          insertIndexes(tableIndexes, keyValue, blockNum);
          
          //setup storage info and write to data
          StorageInfo info(theTable->hashName(), size, kNewBlock, BlockType::data_block, id);
          storage.save(ss, info);
      }
      changed = true;
      return StatusResult{ Errors::noError, affectedRows };
  }  

  std::string Database::getPrimaryKey(std::shared_ptr<Query> aQuery) {
      //get the name of primary key attribute
      for (auto& att : aQuery->getFrom()->getAttributes()) {
          if (att.isPrimaryKey()) {
              return att.getName();
          }
      }
      return "";
  }

  StatusResult Database::selectRows(std::shared_ptr<Query> aQuery, RowCollection& aRows) {
      if (!aQuery)
          return StatusResult{ Errors::unknownCommand };

      auto theId = aQuery->getFrom()->hashName(); //reference ID of the entity
      int count = 0;

      std::string primaryKey = getPrimaryKey(aQuery);

      //find the primary key index
      for (auto& index : indexes) {
          if (index.getTableName() == aQuery->getFrom()->getName() && index.getFieldName() == primaryKey) {
              index.each([&](const Block& theBlock, uint32_t blockIndex)->bool {
                  //read row data
                  std::stringstream ss;
                  ss.write(theBlock.payload, theBlock.header.size);
                  std::unique_ptr<Row> row = std::make_unique<Row>();                  
                  row->decode(ss);

                  //push to the collections if the row matches the query
                  if (aQuery->matches(row->getData())) {
                      aRows.push_back(std::move(row));
                      ++count;
                  }
                  if (count == aQuery->getLimit())
                      return false;

                  return true;
                  }
              );
              break;
          }
      }
      return StatusResult{ Errors::noError };
  }

  static std::ostream& operator<< (std::ostream& out, const Value& aValue) {
      std::visit([&out](auto const& aValue)
      { out <<  aValue; }, aValue);
      return out;
  }

  std::shared_ptr<Query> Database::buildQuery(Join& aJoin, Value aValue) {
      std::shared_ptr<Query> theQuery = std::make_shared<Query>();
      theQuery->setEntityName(aJoin.onRight.tableName);
      theQuery->setSelectAll(true);
      for (auto& entity : entities) {
          if (entity.getName() == theQuery->getEntityName()) {
              theQuery->setFrom(&entity);
              break;
          }
      }

      std::stringstream ss;
      ss << aJoin.onRight.fieldName << " = " << aValue;

      Tokenizer theTokenizer(ss);
      theTokenizer.tokenize();
      theQuery->parseFilters(theTokenizer);

      return theQuery;
  }

  static void buildJoinedRow(std::shared_ptr<Query> aQuery, RowCollection& aCollection, 
      std::unique_ptr<Row> aLeftRow, RowCollection& aRightRows) {
      KeyValues leftData = aLeftRow->getData();
      if (aRightRows.size() == 0) {
          KeyValues keyValue;
          for (auto& attName : aQuery->getSelects()) {
              if (leftData.count(attName)) {
                  keyValue[attName] = leftData[attName];
              }
              else {
                  keyValue[attName] = std::string("NULL");
              }
          }
          std::unique_ptr<Row> theRow = std::make_unique<Row>(keyValue, 0);
          aCollection.push_back(std::move(theRow));
      }
      for (auto& rRow : aRightRows) {
          KeyValues rightData = rRow->getData();
          KeyValues keyValue;
          for (auto& attName : aQuery->getSelects()) {
              if (leftData.count(attName)) {
                  keyValue[attName] = leftData[attName];
              }
              else {
                  keyValue[attName] = rightData[attName];
              }
          }
          std::unique_ptr<Row> theRow = std::make_unique<Row>(keyValue, 0);
          aCollection.push_back(std::move(theRow));
      }
  }

  StatusResult Database::selectJoinRows(std::shared_ptr<Query> aQuery, std::vector<Join> aJoins, 
      RowCollection& aRows) {
      if (!aQuery)
          return StatusResult{ Errors::unknownCommand };

      auto theId = aQuery->getFrom()->hashName(); //reference ID of the entity
      int count = 0;

      std::string primaryKey = getPrimaryKey(aQuery);

      //find the primary key index
      for (auto& index : indexes) {
          if (index.getTableName() == aQuery->getFrom()->getName() && index.getFieldName() == primaryKey) {
              index.each([&](const Block& theBlock, uint32_t blockIndex)->bool {
                  //read row data
                  std::stringstream ss;
                  ss.write(theBlock.payload, theBlock.header.size);
                  std::unique_ptr<Row> row = std::make_unique<Row>();
                  row->decode(ss);

                  for (auto& join : aJoins) {
                      auto theQuery = buildQuery(join, row->getValue(join.onLeft.fieldName));
                      RowCollection rightRows;
                      StatusResult result = selectRows(theQuery, rightRows);
                      if (!result)
                          return result;

                      buildJoinedRow(aQuery, aRows, std::move(row), rightRows);
                  }

                  return true; });
              break;
          }
      }

      return StatusResult{ Errors::noError };
  }

  StatusResult Database::updateRows(std::shared_ptr<Query> aQuery, KeyValues& anUpdates) {
      if (!aQuery)
          return StatusResult{ Errors::unknownCommand };

      auto theId = aQuery->getFrom()->hashName(); //reference ID of the entity
      uint32_t count = 0;

      std::string primaryKey = getPrimaryKey(aQuery);

      for (auto& index : indexes) {
          if (index.getTableName() == aQuery->getFrom()->getName() && index.getFieldName() == primaryKey) {
              index.each([&](const Block& theBlock, uint32_t blockIndex)->bool {
                  //read and decode row data
                  std::stringstream ss;
                  ss.write(theBlock.payload, theBlock.header.size);
                  std::unique_ptr<Row> row = std::make_unique<Row>();
                  row->decode(ss);

                  //update row data if matches the query
                  if (aQuery->matches(row->getData())) {
                      ++count;

                      //update and encode
                      std::stringstream news; //a new stream
                      row->update(anUpdates);
                      row->encode(news);

                      //prepare for a new block
                      Block newBlock = theBlock;
                      newBlock.header.size = news.str().size();
                      news.read(newBlock.payload, newBlock.header.size);

                      //overwrite data
                      storage.writeBlock(blockIndex, newBlock);
                  }
                  return true;
                  }
              );
              break;
          }
      }
      changed = true;
      return StatusResult{ Errors::noError, count };
  }

  StatusResult Database::deleteRows(std::shared_ptr<Query> aQuery) {
      if (!aQuery)
          return StatusResult{ Errors::unknownCommand };

      auto theId = aQuery->getFrom()->hashName(); //reference ID of the entity
      uint32_t count = 0;

      std::string primaryKey = getPrimaryKey(aQuery);
      std::vector<Row*> toBeDelete;

      for (auto& index : indexes) {
          if (index.getTableName() == aQuery->getFrom()->getName() && index.getFieldName() == primaryKey) {
              index.each([&](const Block& theBlock, uint32_t blockIndex)->bool {
                  //read and decode row data
                  std::stringstream ss;
                  ss.write(theBlock.payload, theBlock.header.size);
                  Row* theRow = new Row{};
                  theRow->decode(ss);
                  if (aQuery->matches(theRow->getData()))
                      toBeDelete.push_back(theRow);

                  return true;
                  }
              );
              break;
          }
      }

      for (auto* row : toBeDelete) {
          deleteIndexes(row->getData());
          storage.markBlockAsFree(row->getBlockNum());
          delete row;
      }

      changed = true;
      return StatusResult{ Errors::noError, toBeDelete.size() };
  }

  std::unique_ptr<std::vector<BlockHeader>> Database::debugDump() {
      int blockCount = storage.getBlockCount();
      std::unique_ptr<std::vector<BlockHeader>> res = std::make_unique<std::vector<BlockHeader>>();

      storage.each([&res](Block aBlock, uint32_t aBlockNum) {
          res->push_back(aBlock.header);
          return true; //iterate all blocks, no early exit needed
          }
      );

      return res;
  }

  StatusResult Database::encode(std::ostream &anOutput) {
      anOutput << name << ' ';

      //store table name and corresponding block index
      for (auto& cur : tables) {
          anOutput << cur.first << ' ' << cur.second << ' ';
      }

      //a separator
      anOutput << "# ";

      //store block number of index blocks
      for (auto& cur : indexBlockNums) {          
          anOutput << cur << ' ';
      }

      return StatusResult{Errors::noError};
  }

  StatusResult Database::decode(std::istream &anInput) {
      anInput >> name;

      std::string temp;
      while (anInput >> temp) {
          if (temp == "#")
              break;

          std::string theName = temp;
          anInput >> temp;
          uint32_t theBlockNum = std::stoul(temp);
          tables[theName] = theBlockNum;
      }

      while (anInput >> temp) {
          uint32_t theBlockNum = std::stoul(temp);
          indexBlockNums.insert(theBlockNum);
      }

      return StatusResult{Errors::noError};
  }

}
