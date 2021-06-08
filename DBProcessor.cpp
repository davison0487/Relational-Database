//
//  DBProcessor.hpp
//
//  Created by Yunhsiu Wu on 4/8/21.
//


#include <algorithm>
#include <iostream>
#include <iomanip>
#include <string>
#include <unordered_set>
#include <filesystem>
#include <memory>
#include "Database.hpp"
#include "DBProcessor.hpp"
#include "Tokenizer.hpp"
#include "Timer.hpp"
#include "Config.hpp"
#include "SQLProcessor.hpp"
#include "View.hpp"


namespace ECE141 {

    DBProcessor::DBProcessor(std::ostream& anOutput)
        : CmdProcessor(anOutput), theDB(nullptr), theSQLProc(anOutput) {}

    DBProcessor::~DBProcessor() {
        if(theDB)
            delete theDB;
    }

    static bool isKnown(Keywords aKeyword) {
        static std::unordered_set<Keywords> theKnown{
            Keywords::create_kw, 
            Keywords::drop_kw,
            Keywords::show_kw,
            Keywords::use_kw,
            Keywords::dump_kw
        };

        return theKnown.count(aKeyword);
    }

    static bool isDatabaseKeyword(Keywords aKeyword) {
        static std::unordered_set<Keywords> theKnown{
            Keywords::database_kw,
            Keywords::databases_kw,
        };

        return theKnown.count(aKeyword);
    }

    CmdProcessor* DBProcessor::recognizes(Tokenizer& aTokenizer) {
        if (isKnown(aTokenizer.current().keyword)) {
            if (aTokenizer.current().keyword == Keywords::use_kw || isDatabaseKeyword(aTokenizer.peek().keyword))
                return this;
        }

        return theSQLProc.recognizes(aTokenizer);
    }

    Statement* DBProcessor::makeStatement(Tokenizer& aTokenizer, StatusResult& aResult) {
        //allocate a DBStatement and parse the input
        DBStatement* theStmt = new DBStatement{};
        theStmt->parse(aTokenizer);
        return theStmt;
    }

    StatusResult DBProcessor::createDB(Statement* aStatement) {
        //expecting a DBStatement
        auto* theStatement = static_cast<DBStatement*>(aStatement);

        std::string dbName = theStatement->getName();
        Database newDB(dbName, CreateDB());

        //produce and display output
        View theView(output);
        theView.show( [](std::ostream& anOutput) {
            anOutput << "Query OK, 1 row affected ";
            }
        );

        theTimer.stop();
        theTimer.showElapsedTime(output);

        return StatusResult{ noError };
    }

    StatusResult DBProcessor::dropDB(Statement* aStatement) {
        //expecting a DBStatement
        auto* theStatement = static_cast<DBStatement*>(aStatement);

        std::string dbName = theStatement->getName();
        size_t affectedRows = 0;

        //close database first if we are dropping the opened one
        if (theDB && theDB->getName() == dbName) {
            //tell the SQL processor to stop using the current database
            affectedRows = theDB->getTableNames().size();
            theSQLProc.changeDatabase(nullptr);
            delete theDB;
            theDB = nullptr;
        }
        
        bool result = std::filesystem::remove(Config::getDBPath(dbName));

        //produce and display output
        View theView(output);
        theView.show([&](std::ostream& anOutput) {
            if (result)
                anOutput << "Query OK, " << affectedRows << " row affected ";
                else
                    anOutput << "Query failed, database not found ";
            }
        );

        theTimer.stop();
        theTimer.showElapsedTime(output);
        
        if (!result)
            return StatusResult{ unknownDatabase };

        return StatusResult{ noError };
    }

    StatusResult DBProcessor::showDBs(Statement* aStatement) {        
        //expecting a DBStatement
        //the statment is not required in this method, included for completeness
        auto* theStatement = static_cast<DBStatement*>(aStatement);

        //produce and display output
        DBView theView(output);
        theView.showDBs();

        theTimer.stop();
        theTimer.showElapsedTime(output);

        return StatusResult{ noError };
    }

    StatusResult DBProcessor::useDB(Statement* aStatement) {
        //expecting a DBStatement
        auto* theStatement = static_cast<DBStatement*>(aStatement);

        //close if a database is opened
        std::string dbName = theStatement->getName();
        if (theDB && theDB->getName() != dbName) {
            delete theDB;
            theDB = nullptr;
        }

        //allocate a new database and tell SQL processor to change database        
        if (!theDB) {
            theDB = new Database(dbName, OpenDB());
            theSQLProc.changeDatabase(theDB);
        }
        
        //true if the database is opened successfully
        bool result = theDB; 

        //produce and display output
        View theView(output);
        theView.show([&result](std::ostream& anOutput) {
                if (result)
                    anOutput << "Database changed ";
                else
                    anOutput << "Query failed, database not found ";
            });

        theTimer.stop();
        theTimer.showElapsedTime(output);

        if (!result)
            return StatusResult{ unknownDatabase };

        return StatusResult{ noError };
    }

    StatusResult DBProcessor::debugDump(Statement* aStatement) {
        //expecting a DBStatement
        auto* theStatement = static_cast<DBStatement*>(aStatement);

        //name of the database
        std::string dbName = theStatement->getName();

        std::unique_ptr<std::vector<BlockHeader>> headers;
        if (theDB && theDB->getName() == dbName) {
            //if the target database is currently opened
            headers = theDB->debugDump();
        }
        else {
            //open the target database and read all headers
            Database dumpDatabase(dbName, OpenDB{});
            headers = dumpDatabase.debugDump();
        }

        //produce and display output
        DebugView theView(output);
        theView.debugDump(headers);

        theTimer.stop();
        theTimer.showElapsedTime(output);

        return StatusResult{ noError };
    }

    StatusResult DBProcessor::run(Statement* aStatement, const Timer& aTimer) {
        std::unordered_map<Keywords, std::function<StatusResult()>> theMap{
            {Keywords::create_kw, [&]() {return createDB(aStatement); }},
            {Keywords::drop_kw,   [&]() {return dropDB(aStatement); }},
            {Keywords::show_kw,   [&]() {return showDBs(aStatement); }},
            {Keywords::use_kw,    [&]() {return useDB(aStatement); }},
            {Keywords::dump_kw,   [&]() {return debugDump(aStatement); }}
        };
        
        theTimer = aTimer;

        if (theMap.count(aStatement->getType()))
            return theMap[aStatement->getType()]();

        return StatusResult{ Errors::unexpectedKeyword };
    }
        
}
