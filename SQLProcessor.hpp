//
//  SQLProcessor.hpp
//  RGAssignment3
//
//  Created by rick gessner on 4/1/21.
//

#ifndef SQLProcessor_hpp
#define SQLProcessor_hpp

#include <stdio.h>
#include "CmdProcessor.hpp"
#include "Tokenizer.hpp"
#include "BasicTypes.hpp"
#include "Storage.hpp"
#include "Database.hpp"

namespace ECE141 {

  class Statement;

  class SQLProcessor : public CmdProcessor {
  public:
    
      SQLProcessor(std::ostream& anOutput);
      virtual ~SQLProcessor();
        
      virtual CmdProcessor* recognizes(Tokenizer &aTokenizer) override;
      virtual Statement* makeStatement(Tokenizer &aTokenizer, StatusResult& aResult) override;
      virtual StatusResult run(Statement *aStmt, const Timer& aTimer) override;

      StatusResult createTable(Statement* aStatement);
      StatusResult showTables(Statement* aStatement);
      StatusResult dropTable(Statement* aStatement);
      StatusResult describeTable(Statement* aStatement);
      StatusResult insertRows(Statement* aStatement);
      StatusResult showQuery(Statement* aStatement);
      StatusResult updateTable(Statement* aStatement);
      StatusResult deleteRows(Statement* aStatement);
      StatusResult showIndex(Statement* aStatement);
      StatusResult alterTable(Statement* aStatement);

      void changeDatabase(Database* aDB) { theDB = aDB; }

  protected:
      Database* theDB;

  };

}

#endif /* SQLProcessor_hpp */
