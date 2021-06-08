//
//  Statement.cpp
//  Database
//
//  Created by rick gessner on 3/20/19.
//  Copyright © 2019 rick gessner. All rights reserved.
//

#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <ctime>
#include "Statement.hpp"
#include "Tokenizer.hpp"
#include "Helpers.hpp"

namespace ECE141 {

    Statement::Statement(Keywords aStatementType) : stmtType(aStatementType) {}
    
    Statement::Statement(const Statement &aCopy) : stmtType(aCopy.stmtType) {}
    
    Statement::~Statement() {}
    
    StatusResult Statement::parse(Tokenizer &aTokenizer) {
        return StatusResult{};
    }

    namespace ParseHelper {
        DataTypes keywordsToType(Keywords aKeyword) {
            static std::unordered_map<Keywords, DataTypes> theTypes{
                {Keywords::integer_kw, DataTypes::int_type},
                {Keywords::varchar_kw, DataTypes::varchar_type},
                {Keywords::float_kw, DataTypes::float_type},
                {Keywords::datetime_kw, DataTypes::datetime_type},
                {Keywords::boolean_kw, DataTypes::bool_type},
            };

            if (!theTypes.count(aKeyword))
                return DataTypes::no_type;

            return theTypes[aKeyword];
        }

        //parse a string into boolean
        bool stob(std::string aStr) {
            if (aStr == "0")
                return false;
            else
                return true;
        }
        
        std::string getCurrentTime() {
            std::time_t now = time(0);            
            
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
            std::tm* ltm = new tm{};
            localtime_s(ltm, &now);

#elif __APPLE__ || defined __linux__ || defined __unix__
            tm* ltm = localtime(&now);

#endif

            std::string res; //example output string: 2021-05-08 16:44:50

            res += std::to_string(1900 + ltm->tm_year) + '-';
            res += std::to_string(1 + ltm->tm_mon) + '-';
            res += std::to_string(ltm->tm_mday) + ' ';
            res += std::to_string(ltm->tm_hour) + ':';
            res += std::to_string(ltm->tm_min) + ':';
            res += std::to_string(ltm->tm_sec);

            delete ltm;

            return res;
        }
        
        StatusResult parseFlags(Tokenizer& aTokenizer, Attribute& anAtt) {
            bool flag = aTokenizer.current().data != ")" 
                && aTokenizer.current().data != ","
                && aTokenizer.current().data != ";";

            while (aTokenizer.more() && flag) {
                if (aTokenizer.current().type != TokenType::keyword)
                    return StatusResult{ Errors::keywordExpected };

                //auto increment
                if (aTokenizer.skipIf(Keywords::auto_increment_kw)) {
                    anAtt.setAutoIncrement(true);
                }
                //not null
                else if (aTokenizer.skipIf(Keywords::not_kw)) {
                    if (!aTokenizer.skipIf(Keywords::null_kw))
                        return StatusResult{ Errors::keywordExpected };

                    anAtt.setNullable(false);
                }
                //primary key
                else if (aTokenizer.skipIf(Keywords::primary_kw)) {
                    if (!aTokenizer.skipIf(Keywords::key_kw))
                        return StatusResult{ Errors::keywordExpected };

                    anAtt.setPrimaryKey(true);
                }
                //default
                else if (aTokenizer.skipIf(Keywords::default_kw)) {
                    anAtt.setHasDefault(true);

                    std::unordered_map<DataTypes, std::function<void()>> theMap{
                        { DataTypes::bool_type, 
                            [&]() {anAtt.setDefaultValue(aTokenizer.current().keyword == Keywords::true_kw); }},
                        { DataTypes::datetime_type, 
                            [&]() {anAtt.setDefaultValue(getCurrentTime()); } },
                        { DataTypes::float_type, 
                            [&]() {anAtt.setDefaultValue(std::stod(aTokenizer.current().data)); } },
                        { DataTypes::int_type, 
                            [&]() {anAtt.setDefaultValue(std::stoi(aTokenizer.current().data)); } },
                        { DataTypes::varchar_type, 
                            [&]() {anAtt.setDefaultValue(aTokenizer.current().data.substr(0, anAtt.getLength())); } }
                    };

                    theMap[anAtt.getType()];
                    aTokenizer.next();
                }

                flag = aTokenizer.current().type != TokenType::punctuation;
            }

            return StatusResult{ Errors::noError };
        }

        //parse attribute info
        StatusResult parseAttribute(Tokenizer& aTokenizer, SQLStatement* theStatement) {
            Attribute theAtt;

            if (aTokenizer.current().type != TokenType::identifier)
                return StatusResult{ Errors::identifierExpected };

            //read attribute name
            theAtt.setName(aTokenizer.current().data);
            aTokenizer.next();

            if (aTokenizer.current().type != TokenType::keyword)
                return StatusResult{ Errors::keywordExpected };

            //read data type
            theAtt.setType(keywordsToType(aTokenizer.current().keyword));
            aTokenizer.next();

            //set length if the data type is varchar
            if (theAtt.getType() == DataTypes::varchar_type) {
                //expecting a '('
                if (!aTokenizer.skipIf('('))
                    return StatusResult{ Errors::punctuationExpected };

                theAtt.setLength(std::stoi(aTokenizer.current().data));
                aTokenizer.next();

                //expecting a ')'
                if (!aTokenizer.skipIf(')'))
                    return StatusResult{ Errors::punctuationExpected };
            }

            //flags
            StatusResult result = parseFlags(aTokenizer, theAtt);
            if (!result)
                return result;

            theStatement->addAttribute(theAtt);

            return StatusResult{ Errors::noError };
        }

        //parse row data
        StatusResult parseAttributeData(Tokenizer& aTokenizer, std::vector<std::string>& anAtt) {
            //expecting a '('
            if (!aTokenizer.skipIf('('))
                return StatusResult{ Errors::punctuationExpected };

            bool more = true;
            while (more && aTokenizer.more()) {
                if (aTokenizer.current().type != TokenType::identifier &&
                    aTokenizer.current().type != TokenType::number)
                    return StatusResult{ Errors::valueExpected };

                std::string tmp{ aTokenizer.current().data };
                anAtt.push_back(aTokenizer.current().data);
                aTokenizer.next();

                more = aTokenizer.skipIf(',');
            }

            //expecting a ')'
            if (!aTokenizer.skipIf(')'))
                return StatusResult{ Errors::punctuationExpected };

            return StatusResult{ Errors::noError };
        }
                
    }

    StatusResult DBStatement::parseDBName(Tokenizer& aTokenizer) {
        if (!aTokenizer.skipIf(Keywords::database_kw))
            return StatusResult{ Errors::keywordExpected };
        
        if (aTokenizer.current().type != TokenType::identifier)
            return StatusResult{ Errors::identifierExpected };
        
        dbName = aTokenizer.current().data;
        aTokenizer.next();
        
        return StatusResult{ Errors::noError };        
    }

    StatusResult DBStatement::parseShow(Tokenizer& aTokenizer) {
        if (!aTokenizer.skipIf(Keywords::databases_kw))
            return StatusResult{ Errors::keywordExpected };
        
        return StatusResult{ Errors::noError };
    }

    StatusResult DBStatement::parseUse(Tokenizer& aTokenizer) {
        if (aTokenizer.current().type != TokenType::identifier)
            return StatusResult{ Errors::identifierExpected };
        
        dbName = aTokenizer.current().data;
        aTokenizer.next();

        return StatusResult{ Errors::noError };
    }
    
    StatusResult DBStatement::parse(Tokenizer& aTokenizer) {        
        std::unordered_map<Keywords, std::function<StatusResult()>> theMap{
            {Keywords::create_kw, [&]() {return parseDBName(aTokenizer); }},
            {Keywords::drop_kw,   [&]() {return parseDBName(aTokenizer); }},
            {Keywords::show_kw,   [&]() {return parseShow(aTokenizer); }},
            {Keywords::use_kw,    [&]() {return parseUse(aTokenizer); }},
            {Keywords::dump_kw,   [&]() {return parseDBName(aTokenizer); }}
        };

        stmtType = aTokenizer.current().keyword;
        aTokenizer.next();

        if (theMap.count(stmtType))
            return theMap[stmtType]();

        return  StatusResult{ Errors::unexpectedKeyword };
    }

    StatusResult SQLStatement::parseCreate(Tokenizer& aTokenizer) {        
        if (!aTokenizer.skipIf(Keywords::table_kw))
            return StatusResult{ Errors::keywordExpected };

        if (aTokenizer.current().type != TokenType::identifier)
            return StatusResult{ Errors::identifierExpected };

        //read table name
        tableName = aTokenizer.current().data;
        aTokenizer.next();
                
        //expecting a '('
        if (!aTokenizer.skipIf(TokenType::punctuation))
            return StatusResult{ Errors::punctuationExpected };

        //read attributes
        bool more = true;
        while (aTokenizer.more() && more) {                
                StatusResult result = ParseHelper::parseAttribute(aTokenizer, this);

                if (!result)
                    return result;

                more = aTokenizer.skipIf(','); 
        }

        return StatusResult{ Errors::noError };
    }

    StatusResult SQLStatement::parseShow(Tokenizer& aTokenizer) {
        if (!aTokenizer.skipIf(Keywords::tables_kw))
            return StatusResult{ Errors::keywordExpected };

        return StatusResult{ Errors::noError };
    }

    StatusResult SQLStatement::parseDrop(Tokenizer& aTokenizer) {
        if (!aTokenizer.skipIf(Keywords::table_kw))
            return StatusResult{ Errors::keywordExpected };
        
        if (aTokenizer.current().type != TokenType::identifier)
            return StatusResult{ Errors::identifierExpected };
        
        tableName = aTokenizer.current().data;
        aTokenizer.next();

        return StatusResult{ Errors::noError };
    }

    StatusResult SQLStatement::parseDescribe(Tokenizer& aTokenizer) {
        if (aTokenizer.current().type != TokenType::identifier)
            return StatusResult{ Errors::identifierExpected };
        
        tableName = aTokenizer.current().data;
        aTokenizer.next();

        return StatusResult{ Errors::noError };
    }

    StatusResult SQLStatement::parse(Tokenizer& aTokenizer) {
        std::unordered_map<Keywords, std::function<StatusResult()>> theMap{
            {Keywords::create_kw,   [&]() { return parseCreate(aTokenizer); }},
            {Keywords::show_kw,     [&]() { return parseShow(aTokenizer); }},
            {Keywords::drop_kw,     [&]() { return parseDrop(aTokenizer); }},
            {Keywords::describe_kw, [&]() { return parseDescribe(aTokenizer); }}
        };

        stmtType = aTokenizer.current().keyword;
        aTokenizer.next();

        if (theMap.count(stmtType))
            return theMap[stmtType]();

        return StatusResult{ Errors::unexpectedKeyword };
    }

    StatusResult AlterStatement::parse(Tokenizer& aTokenizer) {
        if (!aTokenizer.skipIf(Keywords::alter_kw))
            return StatusResult{ Errors::keywordExpected };

        if (!aTokenizer.skipIf(Keywords::table_kw))
            return StatusResult{ Errors::keywordExpected };

        if (aTokenizer.current().type != TokenType::identifier)
            return StatusResult{ Errors::identifierExpected };

        tableName = aTokenizer.current().data;
        aTokenizer.next();

        if (aTokenizer.current().keyword != Keywords::add_kw &&
            aTokenizer.current().keyword != Keywords::drop_kw)
            return StatusResult{ Errors::unexpectedKeyword };

        mode = aTokenizer.current().keyword;
        aTokenizer.next();

        if (mode == Keywords::add_kw) {
            StatusResult result = ParseHelper::parseAttribute(aTokenizer, this);
            if (!result)
                return result;
        }
        else {            
            Attribute att{ aTokenizer.current().data, DataTypes::no_type };
            this->addAttribute(att);
        }
        
        return StatusResult{ Errors::noError };
    }
       
    StatusResult InsertStatement::parse(Tokenizer& aTokenizer) {
        if (!aTokenizer.skipIf(Keywords::insert_kw) || !aTokenizer.skipIf(Keywords::into_kw))
            return StatusResult{ Errors::keywordExpected };

        stmtType = Keywords::insert_kw;

        if (aTokenizer.current().type != TokenType::identifier)
            return StatusResult{ Errors::identifierExpected };

        //table name
        entityName = aTokenizer.current().data;
        aTokenizer.next();

        //attribute names
        auto result = ParseHelper::parseAttributeData(aTokenizer, attributes);
        if (!result)
            return result;
                
        if (!aTokenizer.skipIf(Keywords::values_kw))
            return StatusResult{ Errors::keywordExpected };

        //attribute values
        bool more = true;
        while (more) {
            std::vector<std::string> tmp;
            auto result = ParseHelper::parseAttributeData(aTokenizer, tmp);

            if (!result)
                return result;

            values.push_back(tmp);
            more = aTokenizer.skipIf(',');
        }

        /*example presentation of resulting data
        +------------+--------+--------+--------+--------+--------+
        | attributes | att_1  | att_2  | att_3  | att_4  | att_5  |
        +------------+--------+--------+--------+--------+--------+
        | values_1   | data_1 | data_2 | data_3 | data_4 | data_5 |
        | values_2   | data_1 | data_2 | data_3 | data_4 | data_5 |
        | values_3   | data_1 | data_2 | data_3 | data_4 | data_5 |
        | values_4   | data_1 | data_2 | data_3 | data_4 | data_5 |
        | values_5   | data_1 | data_2 | data_3 | data_4 | data_5 |
        | values_6   | data_1 | data_2 | data_3 | data_4 | data_5 |
        +------------+--------+--------+--------+--------+--------+ */

        return StatusResult{ Errors::noError };
    }

    StatusResult SelectStatement::parseSelect(Tokenizer& aTokenizer) {
        if (!aTokenizer.skipIf(Keywords::select_kw))
            return StatusResult{ Errors::keywordExpected };
        
        //select all
        if (aTokenizer.skipIf('*')) {
            theQuery->setSelectAll(true);
            aTokenizer.skipTo(Keywords::from_kw); //skip potential bad inputs
        }
        //select certain fields
        else {
            while (aTokenizer.more() && aTokenizer.current().keyword != Keywords::from_kw) {
                if (aTokenizer.skipIf(','))
                    continue;

                if (aTokenizer.current().type != TokenType::identifier)
                    return StatusResult{ Errors::identifierExpected };

                //add a field
                theQuery->setSelect(aTokenizer.current().data);
                aTokenizer.next();               
            }
        }

        return StatusResult{ Errors::noError };
    }

    StatusResult SelectStatement::parseEntity(Tokenizer& aTokenizer, Database* aDB) {
        if (aTokenizer.current().type != TokenType::identifier)
                return StatusResult{ Errors::identifierExpected };

        //entity name
        std::string entityName = aTokenizer.current().data;
        aTokenizer.next();
        theQuery->setEntityName(entityName);

        if (aDB) {
            Entity* theEntity = aDB->getEntity(entityName);
            if (!theEntity)
                return StatusResult{ Errors::unknownEntity };
            
            theQuery->setFrom(theEntity);
            return StatusResult{ Errors::noError };
        }
        
        return StatusResult{ Errors::unknownDatabase };
    }

    StatusResult SelectStatement::parseOrderBy(Tokenizer& aTokenizer) {        
        bool more = true;
        while (more) {
            if (aTokenizer.current().type != TokenType::identifier)
                return StatusResult{ Errors::identifierExpected };

            std::string field{ aTokenizer.current().data };
            aTokenizer.next();

            if (aTokenizer.skipIf(Keywords::desc_kw)) {
                theQuery->setOrderBy(field, false);
            }
            else {
                //default ordering
                aTokenizer.skipIf(Keywords::asc_kw);
                theQuery->setOrderBy(field, true);
            }

            more = aTokenizer.skipIf(',');
        }
        
        return StatusResult{ Errors::noError };
    }

    static std::unordered_set<Keywords> joinKeywords{ ECE141::Keywords::cross_kw, 
        ECE141::Keywords::full_kw, ECE141::Keywords::inner_kw, ECE141::Keywords::left_kw,
        ECE141::Keywords::right_kw, ECE141::Keywords::join_kw };

    static StatusResult parseTableName(Tokenizer& aTokenizer, std::string& aTableName) {
        if (aTokenizer.current().type != TokenType::identifier)
            return StatusResult{ Errors::identifierExpected };

        aTableName = aTokenizer.current().data;
        aTokenizer.next();
        return StatusResult{ Errors::noError };
    }
    
    static StatusResult parseTableField(Tokenizer& aTokenizer, TableField& aTableField) {
        aTableField.tableName = aTokenizer.current().data;
        aTokenizer.next();

        if (!aTokenizer.skipIf('.'))
            return StatusResult{ Errors::syntaxError };

        aTableField.fieldName = aTokenizer.current().data;
        aTokenizer.next();

        return StatusResult{ Errors::noError };
    }

    StatusResult SelectStatement::convertToRightJoin(Database* aDB, std::string& aTable,
        TableField& aLhs, TableField& aRhs) {
        //convert query entity to the right table
        std::string rightTable = aRhs.tableName;
        theQuery->setEntityName(rightTable);
        Entity* theEntity = aDB->getEntity(rightTable);
        theQuery->setFrom(theEntity);
        if (!theEntity)
            return StatusResult{ Errors::unknownEntity };

        //swap lhs and rhs
        TableField temp{ aLhs };
        aLhs = aRhs;
        aRhs = temp;

        return StatusResult{ Errors::noError };
    }

    StatusResult SelectStatement::parseJoin(Tokenizer& aTokenizer, Database* aDB) {
        StatusResult theResult{ Errors::noError };

        Keywords theJoinType{ Keywords::join_kw };
        if (aTokenizer.current().keyword == Keywords::join_kw) {
            //plain join is equivalent to inner join
            theJoinType = Keywords::inner_kw;
        }
        else {
            theJoinType = aTokenizer.current().keyword;
            aTokenizer.next();
        }

        if (!aTokenizer.skipIf(Keywords::join_kw))
            return StatusResult{ Errors::keywordExpected };
                
        std::string theTable;
        if ((theResult = parseTableName(aTokenizer, theTable))) {
            if (!aTokenizer.skipIf(Keywords::on_kw))
                return StatusResult{ Errors::keywordExpected };

            TableField lhs, rhs;

            theResult = parseTableField(aTokenizer, lhs);
            if (!theResult)
                return theResult;

            if (!aTokenizer.skipIf('='))
                return StatusResult{ Errors::keywordExpected };

            theResult = parseTableField(aTokenizer, rhs);
            if (!theResult)
                return theResult;

            if (theJoinType == Keywords::right_kw) {
                theResult = convertToRightJoin(aDB, theTable, lhs, rhs);
                if (!theResult)
                    return theResult;
            }

            joins.push_back(Join{ theTable, theJoinType, lhs, rhs });
        }

        return theResult;
    }

    StatusResult SelectStatement::parseFlags(Tokenizer& aTokenizer) {
        StatusResult theResult = { Errors::noError };

        while (aTokenizer.more() && aTokenizer.current().data != ";") {
            //where clause
            if (aTokenizer.skipIf(Keywords::where_kw)) {
                theResult = theQuery->parseFilters(aTokenizer);
                if (!theResult)
                    return theResult;
            }
            //order by clause
            else if (aTokenizer.skipIf(Keywords::order_kw) && aTokenizer.skipIf(Keywords::by_kw)) {
                theResult = parseOrderBy(aTokenizer);
                if (!theResult)
                    return theResult;
            }
            //limit clause
            else if (aTokenizer.skipIf(Keywords::limit_kw)) {
                if (aTokenizer.current().type != TokenType::number)
                    return StatusResult{ Errors::valueExpected };

                theQuery->setLimit(std::stoi(aTokenizer.current().data));
            }
            //unknown clause, force forward
            else {
                aTokenizer.next();
            }
        }

        aTokenizer.next();
        return theResult;
    }

    StatusResult SelectStatement::parse(Tokenizer& aTokenizer, Database* aDB) {
        stmtType = Keywords::select_kw;
        theQuery = std::make_shared<Query>();
        StatusResult theResult = { Errors::noError };

        //select clause
        theResult = parseSelect(aTokenizer);
        if (!theResult)
                return theResult;
        
        //from clause
        if (!aTokenizer.skipIf(Keywords::from_kw))
            return StatusResult{ Errors::keywordExpected };
        theResult = parseEntity(aTokenizer, aDB);
        if (!theResult)
            return theResult;

        while (aTokenizer.more()) {
            if (joinKeywords.count(aTokenizer.current().keyword)) {
                //join clause
                theResult = parseJoin(aTokenizer, aDB);
            }
            else {
                //flags
                theResult = parseFlags(aTokenizer);
            }
            if (!theResult)
                return theResult;
        }

        return StatusResult{ Errors::noError };
    }

    StatusResult UpdateStatement::parseSet(Tokenizer& aTokenizer) {
        Entity* theEntity = theQuery->getFrom();
        while (aTokenizer.current().type != TokenType::keyword) {
            if (aTokenizer.current().type != TokenType::identifier)
                return StatusResult{ Errors::identifierExpected };
            
            //read attribute and check if it exists
            std::string attName = aTokenizer.current().data;
            aTokenizer.next();
            Attribute* theAtt = theEntity->getAttribute(attName);
            if (!theAtt)
                return StatusResult{ Errors::unknownAttribute };

            //expecting a '='
            if (!aTokenizer.skipIf('='))
                return StatusResult{ Errors::operatorExpected };

            //read value
            if (aTokenizer.current().type != TokenType::identifier &&
                aTokenizer.current().type != TokenType::number)
                return StatusResult{ Errors::unexpectedValue };

            std::string value = aTokenizer.current().data;
            aTokenizer.next();

            std::unordered_map<DataTypes, std::function<void()>> theMap{
                { DataTypes::bool_type,     [&]() { updates.insert({attName, ParseHelper::stob(value)}); }},
                { DataTypes::datetime_type, [&]() { updates.insert({attName, value}); }},
                { DataTypes::float_type,    [&]() { updates.insert({attName, std::stod(value)}); }},
                { DataTypes::int_type,      [&]() { updates.insert({attName, std::stoi(value)}); }},
                { DataTypes::varchar_type,  [&]() { updates.insert({attName, value}); }}
            };
            
            theMap[theAtt->getType()]();
        }

        return StatusResult{ Errors::noError };
    }

    StatusResult UpdateStatement::parse(Tokenizer& aTokenizer, Database* aDB) {        
        stmtType = Keywords::update_kw;
        theQuery = std::make_shared<Query>();
        StatusResult theResult = { Errors::noError };

        //update clause
        if (!aTokenizer.skipIf(Keywords::update_kw))
            return StatusResult{ Errors::keywordExpected };

        theResult = parseEntity(aTokenizer, aDB);
        if (!theResult)
            return theResult;

        //set clause
        if (!aTokenizer.skipIf(Keywords::set_kw))
            return StatusResult{ Errors::keywordExpected };

        theResult = parseSet(aTokenizer);
        if (!theResult)
            return theResult;

        //where clause
        while (aTokenizer.more() && aTokenizer.current().data != ";") {
            if (aTokenizer.skipIf(Keywords::where_kw)) {
                theResult = theQuery->parseFilters(aTokenizer);
                if (!theResult)
                    return theResult;
            }
            else {
                aTokenizer.next();
            }
        }

        return StatusResult{ Errors::noError };
    }

    StatusResult DeleteStatement::parse(Tokenizer& aTokenizer, Database* aDB) {
        //DELETE FROM Users WHERE zipcode>92000;
        stmtType = Keywords::delete_kw;
        theQuery = std::make_shared<Query>();
        StatusResult theResult = { Errors::noError };

        //delete fron clause
        if (!aTokenizer.skipIf(Keywords::delete_kw))
            return StatusResult{ Errors::keywordExpected };

        if (!aTokenizer.skipIf(Keywords::from_kw))
            return StatusResult{ Errors::keywordExpected };

        theResult = parseEntity(aTokenizer, aDB);
        if (!theResult)
            return theResult;

        //where clause
        while (aTokenizer.more() && aTokenizer.current().data != ";") {
            if (aTokenizer.skipIf(Keywords::where_kw)) {
                theResult = theQuery->parseFilters(aTokenizer);
                if (!theResult)
                    return theResult;
            }
            else {
                aTokenizer.next();
            }
        }

        return StatusResult{ Errors::noError };
    }

    StatusResult IndexStatement::parse(Tokenizer& aTokenizer) {
        if (!aTokenizer.skipIf(Keywords::show_kw)) 
            return StatusResult{ Errors::keywordExpected };

        if (aTokenizer.skipIf(Keywords::indexes_kw)) {
            all = true;
            return StatusResult{ Errors::noError };
        }

        if (!aTokenizer.skipIf(Keywords::index_kw))
            return StatusResult{ Errors::keywordExpected };

        //field names
        while (aTokenizer.more() && aTokenizer.current().keyword != Keywords::from_kw) {
            fieldNames.push_back(aTokenizer.current().data);
            aTokenizer.next();
        }

        if (!aTokenizer.skipIf(Keywords::from_kw))
            return StatusResult{ Errors::keywordExpected };

        //table name
        if (aTokenizer.current().type != TokenType::identifier)
            return StatusResult{ Errors::identifierExpected };        
        tableName = aTokenizer.current().data;
        aTokenizer.next();

        return StatusResult{ Errors::noError };
    }

}
