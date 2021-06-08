//
//  Database.hpp
//  RGAssignment2
//
//  Created by rick gessner on 2/27/21.
//

#ifndef Database_hpp
#define Database_hpp

#include <stdio.h>
#include <fstream> 
#include <map>
#include <memory>
#include <vector>
#include <string>
#include "BasicTypes.hpp"
#include "Storage.hpp"
#include "Attribute.hpp"
#include "Entity.hpp"
#include "Query.hpp"
#include "Index.hpp"
#include "Join.hpp"

namespace ECE141 {

  class Database : public Storable {
  public:
    
    Database(const std::string aName, CreateDB);
    Database(const std::string aName, OpenDB);
    virtual ~Database();

    std::string getName() { return this->name; }

    //get certain entity
    Entity* getEntity(std::string aName);

    //get all table name
    std::vector<std::string> getTableNames();

    //get pair(key / blockNum) of certain index
    IndexPairs getIndex(std::string aTableName, std::vector<std::string> aFields);

    //get pair(table / field) of all indexes
    IndexPairs getAllIndexes();

    void deleteIndexes(KeyValues& aKeyValue);
    void deleteAllIndexes(std::string aTableName);
    void insertIndexes(std::vector<Index*> anIndexes, KeyValues& aKeyValue, uint32_t blockNum);

    StatusResult addTable(std::string aName, const std::vector<Attribute>& anAttributes);
    StatusResult dropTable(std::string aName);
    StatusResult alterTable(std::string aTableName, Keywords aMode, Attribute& anAtt);
    
    std::string getPrimaryKey(std::shared_ptr<Query> aQuery);

    std::shared_ptr<Query> buildQuery(Join& aJoin, Value aValue);

    StatusResult insertRows(std::string aTableName, const std::vector<std::string>& anAttNames, const std::vector<std::vector<std::string>>& aValues);
    StatusResult selectRows(std::shared_ptr<Query> aQuery, RowCollection& aRows);
    StatusResult selectJoinRows(std::shared_ptr<Query> aQuery, std::vector<Join> aJoins, RowCollection& aRows);
    StatusResult updateRows(std::shared_ptr<Query> aQuery, KeyValues& anUpdates);
    StatusResult deleteRows(std::shared_ptr<Query> aQuery);
    
    std::unique_ptr<std::vector<BlockHeader>> debugDump();

    /*----------------Storable----------------*/
    StatusResult    encode(std::ostream &anOutput) override;
    StatusResult    decode(std::istream &anInput) override;
    /*----------------Storable----------------*/

  private:
      StatusResult alterRow(Attribute& anAtt, Keywords aMode, std::string aTableName, std::string aPrimaryKey);      

  protected:    
    std::string         name;
    Storage             storage;
    bool                changed;
    std::fstream        stream; //stream storage uses for IO
        
    NamedIndex          tables; //key:table name, value: block number
    std::vector<Entity> entities; //vector of entities

    std::set<uint32_t>  indexBlockNums; //block number of index blocks
    std::vector<Index>  indexes; //vector of indexes
  };

}
#endif /* Database_hpp */
