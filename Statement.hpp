//
//  Statement.hpp
//  Database
//
//  Created by rick gessner on 3/20/19.
//  Copyright © 2019 rick gessner. All rights reserved.
//

#ifndef Statement_hpp
#define Statement_hpp

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include "keywords.hpp"
#include "Attribute.hpp"
#include "Database.hpp"
#include "Row.hpp"
#include "Query.hpp"
#include "Join.hpp"

namespace ECE141 {
  
  class Tokenizer;
  class CmdProcessor;
  
  class Statement {
  public:
    Statement(Keywords aStatementType = Keywords::unknown_kw);
    Statement(const Statement &aCopy);
    
    virtual               ~Statement();
    
    virtual StatusResult  parse(Tokenizer &aTokenizer);
    
    Keywords              getType() const {return stmtType;}
    
    //virtual StatusResult  dispatch() {return StatusResult{};}

    virtual std::string getName() { return ""; }

  protected:
    Keywords   stmtType;
  };

  //Statement for database processor
  class DBStatement : public Statement {
  public:
      DBStatement(Keywords aStatementType = Keywords::unknown_kw, std::string aName = "")
          : Statement(aStatementType), dbName(aName) {}

      DBStatement(const DBStatement& aCopy)
          : Statement(aCopy.stmtType), dbName(aCopy.dbName) {}

      ~DBStatement() {}
      
      virtual StatusResult parse(Tokenizer& aTokenizer);

      virtual std::string getName() { return dbName; }

  protected:
      //read database name, this method is for create, drop and dump
      StatusResult parseDBName(Tokenizer& aTokenizer);

      //for show
      StatusResult parseShow(Tokenizer& aTokenizer);

      //for use
      StatusResult parseUse(Tokenizer& aTokenizer);
      

      std::string dbName;
  };

  //statement for SQL processor
  class SQLStatement : public Statement {
  public:
      SQLStatement(Keywords aStatementType = Keywords::unknown_kw) :
          Statement(aStatementType), tableName("") {}

      ~SQLStatement() {}

      virtual StatusResult parse(Tokenizer& aTokenizer);

      virtual std::string getName() { return tableName; }

      const AttributeList getAttributes() { return attributes; }

      void addAttribute(Attribute& anAttribute) { attributes.push_back(anAttribute); }

  protected:
      StatusResult parseCreate(Tokenizer& aTokenizer);
      StatusResult parseShow(Tokenizer& aTokenizer);
      StatusResult parseDrop(Tokenizer& aTokenizer);
      StatusResult parseDescribe(Tokenizer& aTokenizer);

      std::string tableName;
      AttributeList attributes;

  };

  class AlterStatement : public SQLStatement {
  public:
      AlterStatement() : SQLStatement(Keywords::alter_kw),
          mode(Keywords::add_kw) {}

      virtual ~AlterStatement() {}

      virtual StatusResult  parse(Tokenizer& aTokenizer);

      Keywords getMode() { return mode; }

  private:
      Keywords mode;
  };
  
  class InsertStatement : public Statement {
  public:
      InsertStatement() : Statement(Keywords::unknown_kw) {}

      ~InsertStatement() {}

      virtual StatusResult parse(Tokenizer& aTokenizer);

      virtual std::string getName() { return entityName; }

      std::vector<std::string>& getAttributes() { return attributes; }
      std::vector<std::vector<std::string>>& getValues() { return values; }

  protected:
      std::string entityName;
      std::vector<std::string> attributes;
      std::vector<std::vector<std::string>> values;

  };
  
  class SelectStatement : public Statement {
  public:
      SelectStatement() : Statement(Keywords::unknown_kw), theQuery(nullptr) {}

      ~SelectStatement() {}

      virtual StatusResult parse(Tokenizer& aTokenizer, Database* aDB);

      std::shared_ptr<Query>& getQuery() { return theQuery; }
      std::vector<Join>& getJoins() { return joins; }

  protected:
      StatusResult parseSelect(Tokenizer& aTokenizer);
      StatusResult parseEntity(Tokenizer& aTokenizer, Database* aDB);
      StatusResult parseOrderBy(Tokenizer& aTokenizer);
      StatusResult convertToRightJoin(Database* aDB, std::string& aTable, TableField& aLhs, TableField& aRhs);
      StatusResult parseJoin(Tokenizer& aTokenizer, Database* aDB);
      StatusResult parseFlags(Tokenizer& aTokenizer);

      std::vector<Join> joins;
      std::shared_ptr<Query> theQuery;
  };

  class UpdateStatement : public SelectStatement {
  public:
      UpdateStatement() : SelectStatement() {}

      ~UpdateStatement() {}

      virtual StatusResult parse(Tokenizer& aTokenizer, Database* aDB);

      StatusResult parseSet(Tokenizer& aTokenizer);

      KeyValues& getUpdates() { return updates; }

  protected:
      KeyValues updates;
  };

  class DeleteStatement : public SelectStatement {
  public:
      DeleteStatement() : SelectStatement() {}

      ~DeleteStatement() {}

      virtual StatusResult parse(Tokenizer& aTokenizer, Database* aDB);

  };

  class IndexStatement : public Statement {
  public:
      IndexStatement() : Statement(Keywords::index_kw),
          all(false), tableName("") {}

      virtual ~IndexStatement() {}
      
      virtual StatusResult  parse(Tokenizer& aTokenizer);

      bool showAll() { return all; }

      std::string getTableName() { return tableName; }

      std::vector<std::string> getFieldNames() { return fieldNames; }

  private:
      bool all;
      std::string tableName;
      std::vector<std::string> fieldNames;
  };
    
}

#endif /* Statement_hpp */
