//
//  DBProcessor.hpp
//
//  Created by Yunhsiu Wu on 4/8/21.
//

#ifndef DBProcessor_hpp
#define DBProcessor_hpp

#include <string>
#include <iostream>
#include "CmdProcessor.hpp"
#include "Database.hpp"
#include "SQLProcessor.hpp"

namespace ECE141 {

    class DBProcessor : public CmdProcessor {
    public:

        DBProcessor(std::ostream& anOutput);
        virtual ~DBProcessor();

        StatusResult createDB(Statement* aStatement);
        StatusResult dropDB(Statement* aStatement);
        StatusResult showDBs(Statement* aStatement);
        StatusResult useDB(Statement* aStatement);
        StatusResult debugDump(Statement* aStatement);

        virtual CmdProcessor* recognizes(Tokenizer& aTokenizer) override;
        virtual Statement* makeStatement(Tokenizer& aTokenizer, StatusResult& aResult) override;
        virtual StatusResult run(Statement* aStmt, const Timer& aTimer) override;

    protected:
        Database *theDB;
        SQLProcessor theSQLProc;
    };

}

#endif /* DBProcessor_hpp */
