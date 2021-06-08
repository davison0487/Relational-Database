//
//  SQLProcessor.cpp
//  RGAssignment3
//
//  Created by rick gessner on 4/1/21.
//

#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <string>
#include <functional>
#include "SQLProcessor.hpp"
#include "Application.hpp"
#include "Database.hpp"
#include "Helpers.hpp"
#include "TabularView.hpp"
#include "Timer.hpp"
#include "DBProcessor.hpp"
#include "View.hpp"
#include "Query.hpp"
#include "Row.hpp"

namespace ECE141 {

    SQLProcessor::SQLProcessor(std::ostream& anOutput)
        : CmdProcessor(anOutput), theDB(nullptr) {}

    SQLProcessor::~SQLProcessor() {}

    static bool isKnown(Keywords aKeyword) {
        static std::unordered_set<Keywords> theKnown{
            Keywords::create_kw,
            Keywords::drop_kw,
            Keywords::show_kw
        };
        
        return theKnown.count(aKeyword);
    }

    static bool isSQLKeyword(Keywords aKeyword) {
        static std::unordered_set<Keywords> theKnown{
            Keywords::table_kw,
            Keywords::tables_kw,
            Keywords::into_kw,
            Keywords::describe_kw,
            Keywords::insert_kw,
            Keywords::select_kw,
            Keywords::update_kw,
            Keywords::delete_kw,
            Keywords::index_kw,
            Keywords::indexes_kw,
            Keywords::alter_kw
        };

        return theKnown.count(aKeyword);
    }

    CmdProcessor* SQLProcessor::recognizes(Tokenizer& aTokenizer) {
        if (isSQLKeyword(aTokenizer.current().keyword))
            return this;

        if (isKnown(aTokenizer.current().keyword)) {
            if (isSQLKeyword(aTokenizer.peek().keyword))
                return this;
        }

        return nullptr;
    }

    namespace StatementFactory {
        Statement* SQLStatmentFactory(Tokenizer& aTokenizer) {
            //allocate a SQLStatement and parse the input
            SQLStatement* theStmt = new SQLStatement{};
            theStmt->parse(aTokenizer);
            return theStmt;
        }

        Statement* ShowStatementFactory(Tokenizer& aTokenizer) {
            //allocate a Statement and parse the input
            Statement* theStmt;
            if (aTokenizer.peek().keyword == Keywords::tables_kw) {
                //show tables
                theStmt = new SQLStatement{};
            }
            else {
                //show index/indexes
                theStmt = new IndexStatement{};
            }

            theStmt->parse(aTokenizer);
            return theStmt;
        }

        Statement* InsertStatmentFactory(Tokenizer& aTokenizer) {
            //allocate a InsertStatement and parse the input
            InsertStatement* theStmt = new InsertStatement{};
            theStmt->parse(aTokenizer);
            return theStmt;
        }

        Statement* SelectStatmentFactory(Tokenizer& aTokenizer, Database* aDB) {
            //allocate a SelectStatement and parse the input
            SelectStatement* theStmt = new SelectStatement{};
            theStmt->parse(aTokenizer, aDB);
            return theStmt;
        }

        Statement* UpdateStatementFactory(Tokenizer& aTokenizer, Database* aDB) {
            //allocate a SelectStatement and parse the input
            UpdateStatement* theStmt = new UpdateStatement{};
            theStmt->parse(aTokenizer, aDB);
            return theStmt;
        }

        Statement* DeleteStatementFactory(Tokenizer& aTokenizer, Database* aDB) {
            //allocate a DeleteStatement and parse the input
            DeleteStatement* theStmt = new DeleteStatement{};
            theStmt->parse(aTokenizer, aDB);
            return theStmt;
        }

        Statement* AlterStatementFactory(Tokenizer& aTokenizer) {
            //allocate a AlterStatement and parse the input
            AlterStatement* theStmt = new AlterStatement{};
            theStmt->parse(aTokenizer);
            return theStmt;
        }
    }
    
    Statement* SQLProcessor::makeStatement(Tokenizer& aTokenizer, StatusResult& aResult) {
        std::unordered_map<Keywords, std::function<Statement* ()>> theFactories{
            {Keywords::create_kw,   [&]() { return StatementFactory::SQLStatmentFactory(aTokenizer); }},            
            {Keywords::drop_kw,     [&]() { return StatementFactory::SQLStatmentFactory(aTokenizer); }},
            {Keywords::describe_kw, [&]() { return StatementFactory::SQLStatmentFactory(aTokenizer); }},
            {Keywords::show_kw,     [&]() { return StatementFactory::ShowStatementFactory(aTokenizer); }},
            {Keywords::insert_kw,   [&]() { return StatementFactory::InsertStatmentFactory(aTokenizer); }},
            {Keywords::select_kw,   [&]() { return StatementFactory::SelectStatmentFactory(aTokenizer, theDB); }},
            {Keywords::update_kw,   [&]() { return StatementFactory::UpdateStatementFactory(aTokenizer, theDB); }},
            {Keywords::delete_kw,   [&]() { return StatementFactory::DeleteStatementFactory(aTokenizer, theDB); }},
            {Keywords::alter_kw,    [&]() { return StatementFactory::AlterStatementFactory(aTokenizer); }}
        };

        return theFactories[aTokenizer.current().keyword]();
    }

    StatusResult SQLProcessor::createTable(Statement* aStatement) {
        //expecting a SQL Statement
        auto* theStatement = static_cast<SQLStatement*>(aStatement);

        StatusResult result = theDB->addTable(theStatement->getName(), theStatement->getAttributes());

        //produce and display output
        View theView(output);
        theView.show([&result](std::ostream& anOutput) {
            if (result)
                anOutput << "Query OK, " << "0 rows affected ";
            else
                anOutput << "Query failed, table already exists ";
            });
        
        theTimer.stop();
        theTimer.showElapsedTime(output);
        
        return result;
    }

    StatusResult SQLProcessor::showTables(Statement* aStatement) {
        //expecting a SQL Statement
        //the statment is not required in this method, included for completeness
        auto* theStatement = static_cast<SQLStatement*>(aStatement);
        
        //produce and display output
        TableView theView(output);
        theView.showTables(theDB->getName(), theDB->getTableNames());

        theTimer.stop();
        theTimer.showElapsedTime(output);

        return StatusResult{ Errors::noError };
    }

    StatusResult SQLProcessor::dropTable(Statement* aStatement) {
        //expecting a SQL Statement
        auto* theStatement = static_cast<SQLStatement*>(aStatement);

        StatusResult result = theDB->dropTable(theStatement->getName());

        //produce and display output
        View theView(output);
        theView.show([&result](std::ostream& anOutput) {
            if (result)
                anOutput << "Query OK, 0 rows affected ";
            else
                anOutput << "Query failed, table not found ";
            });

        theTimer.stop();
        theTimer.showElapsedTime(output);

        return result;
    }

    StatusResult SQLProcessor::describeTable(Statement* aStatement) {
        //expecting a SQL Statement
        auto* theStatement = static_cast<SQLStatement*>(aStatement);

        //retrieve the entity and check if it exists
        Entity* theEntity = theDB->getEntity(theStatement->getName());
        bool result = theEntity;

        //produce and display output
        DescribeView theView(output);
        if (result)
            theView.describeTable(theEntity);
        else {
            theView.show([](std::ostream& anOutput) {
                anOutput << "Query failed, table not found ";
                });
        }

        theTimer.stop();
        theTimer.showElapsedTime(output);

        if (!result)
            return StatusResult{ Errors::unknownTable };

        return StatusResult{ Errors::noError };
    }

    StatusResult SQLProcessor::insertRows(Statement* aStatement) {
        //expecting an Insert Statement
        auto* theStatement = static_cast<InsertStatement*>(aStatement);

        StatusResult result = theDB->insertRows(theStatement->getName(), theStatement->getAttributes(), theStatement->getValues());        

        //produce and display output
        View theView(output);
        if (result) {
            theView.show([&](std::ostream& anOutput) {
                anOutput << "Query OK, " << result.value << " rows affected ";
                });
        }
        else {
            theView.show([](std::ostream& anOutput) {
                anOutput << "Query failed ";
                });
        }
        
        theTimer.stop();
        theTimer.showElapsedTime(output);

        return result;
    }

    StatusResult SQLProcessor::showQuery(Statement* aStatement) {
        //expecting a Select Statement
        auto* theStatement = static_cast<SelectStatement*>(aStatement);
        std::shared_ptr<Query> theQuery = theStatement->getQuery();
        std::vector<Join> joins = theStatement->getJoins();

        //select rows the matches the query or join
        StatusResult result{ Errors::noError };
        RowCollection collection;
        if (joins.size())
            result = theDB->selectJoinRows(theQuery, joins, collection);
        else
            result = theDB->selectRows(theQuery, collection);

        //produce and display output
        QueryView theView(output);
        if (result) {
            theView.showQuery(theQuery, collection);
        }
        else {
            theView.show([](std::ostream& anOutput) {
                anOutput << "Error occur when selecting rows! ";
                });
        }

        theTimer.stop();
        theTimer.showElapsedTime(output);

        return result;
    }

    StatusResult SQLProcessor::updateTable(Statement* aStatement) {
        //expecting an Update Statement
        auto* theStatement = static_cast<UpdateStatement*>(aStatement);

        StatusResult result = theDB->updateRows(theStatement->getQuery(), theStatement->getUpdates());

        //produce and display output
        View theView(output);
        if (result) {
            theView.show([&](std::ostream& anOutput) {
                anOutput << "Query Ok. "
                    << result.value
                    << " rows affected ";
                });
        }
        else {
            theView.show([](std::ostream& anOutput) {
                anOutput << "Error occur when updating table! ";
                });
        }

        theTimer.stop();
        theTimer.showElapsedTime(output);

        return result;
    }
    
    StatusResult SQLProcessor::deleteRows(Statement* aStatement) {
        //expecting an Delete Statement
        auto* theStatement = static_cast<DeleteStatement*>(aStatement);

        StatusResult result = theDB->deleteRows(theStatement->getQuery());

        //produce and display output
        View theView(output);
        if (result) {
            theView.show([&](std::ostream& anOutput) {
                anOutput << "Query Ok. "
                    << result.value
                    << " rows affected ";
                });
        }
        else {
            theView.show([](std::ostream& anOutput) {
                anOutput << "Error occur when deleting rows! ";
                });
        }

        theTimer.stop();
        theTimer.showElapsedTime(output);

        return result;
    }

    StatusResult SQLProcessor::showIndex(Statement* aStatement) {
        //expecting an Index Statement
        auto* theStatement = static_cast<IndexStatement*>(aStatement);
        
        StatusResult result{ Errors::noError };
        IndexView theView(output);

        if (theStatement->showAll()) {
            auto thePairs = theDB->getAllIndexes();
            theView.showPairs(thePairs, true);
        }
        else {
            auto thePairs = theDB->getIndex(theStatement->getTableName(), theStatement->getFieldNames());
            theView.showPairs(thePairs, false);
        }

        theTimer.stop();
        theTimer.showElapsedTime(output);

        return result;
    }

    StatusResult SQLProcessor::alterTable(Statement* aStatement) {
        //expecting an Alter Statement
        auto* theStatement = static_cast<AlterStatement*>(aStatement);
        Attribute theAtt = theStatement->getAttributes()[0];

        StatusResult result = theDB->alterTable(theStatement->getName(), theStatement->getMode(), theAtt);

        //produce and display output
        View theView(output);
        if (result) {
            theView.show([&](std::ostream& anOutput) {
                anOutput << "Query Ok. 0 rows affected ";
                });
        }
        else {
            theView.show([](std::ostream& anOutput) {
                anOutput << "Error occur when altering table! ";
                });
        }

        theTimer.stop();
        theTimer.showElapsedTime(output);

        return result;
    }

    StatusResult SQLProcessor::run(Statement* aStatement, const Timer& aTimer) {
        std::unordered_map<Keywords, std::function<StatusResult()>> theMap{
            {Keywords::create_kw,   [&]() { return createTable(aStatement); }},
            {Keywords::show_kw,     [&]() { return showTables(aStatement); }},
            {Keywords::drop_kw,     [&]() { return dropTable(aStatement); }},
            {Keywords::describe_kw, [&]() { return describeTable(aStatement); }},
            {Keywords::insert_kw,   [&]() { return insertRows(aStatement); }},
            {Keywords::select_kw,   [&]() { return showQuery(aStatement); }},
            {Keywords::update_kw,   [&]() { return updateTable(aStatement); }},
            {Keywords::delete_kw,   [&]() { return deleteRows(aStatement); }},
            {Keywords::index_kw,    [&]() { return showIndex(aStatement); }},
            {Keywords::alter_kw,    [&]() { return alterTable(aStatement); }}
        };

        theTimer = aTimer;

        if (theMap.count(aStatement->getType()))
            return theMap[aStatement->getType()]();

        return StatusResult{ Errors::unexpectedKeyword };
    }
}
