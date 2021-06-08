//
//  DBView.cpp
//  Assignment3
//
//  Created by Yunhsiu Wu on 4/18/21.
//

#include <iomanip>
#include <filesystem>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include "View.hpp"

namespace ECE141 {

    using OrderedRow = std::map<std::string, RowCollection>;

    bool View::show(ShowResult aShow) {
        aShow(output);
        return true;
    }

    void DBView::showDBs() {
        //setting up the header
        output
            << "+--------------------+\n"
            << "| Database           |\n"
            << "+--------------------+\n";

        //number of total databases
        int count = 0;

        //iterate the database directory
        for (auto& cur : std::filesystem::directory_iterator(path)) {
            if (cur.path().extension() == this->extension) {
                ++count;
                output << "| " << std::left << std::setw(19)
                    << cur.path().stem() << "|\n";
            }
        }

        output << "+--------------------+\n"
            << count << " rows in set ";
    }

    void TableView::showTables(std::string aName, std::vector<std::string> aTables) {
        output
            << "+--------------------------+\n"
            << "| Tables in " << std::setw(15) << std::left << aName << "|\n"
            << "+--------------------------+\n";

        for (auto& table : aTables) {
            output << "| "
                << std::setw(25) << std::left
                << table << "|\n";
        }

        output << "+--------------------------+\n"
            << aTables.size() << " rows in set ";
    }

    //helper: create output string for given data type
    static std::string typeToString(DataTypes aType, int aLength) {
        static std::unordered_map< DataTypes, std::string> theMap{
            {DataTypes::bool_type, "boolean"},
            {DataTypes::datetime_type, "date"},
            {DataTypes::float_type, "float"},
            {DataTypes::int_type, "integer"},
            {DataTypes::varchar_type, "varchar"},
        };
        
        std::string res = theMap[aType];

        if (aType == DataTypes::varchar_type)
            res += "(" + std::to_string(aLength) + ")";

        return res;
    }

    static std::ostream& operator<< (std::ostream& out, const Value& aValue) {
        std::visit([&out](auto const& aValue)
        { out << aValue; }, aValue);
        return out;
    }

    bool DescribeView::describeTable(Entity* anEntity) {
        if (!anEntity)
            return false;

        output
            << "+---------------------+--------------+------+-----+---------+-----------------------------+\n"
            << "| Field               | Type         | Null | Key | Default | Extra                       |\n"
            << "+---------------------+--------------+------+-----+---------+-----------------------------+\n";
        
        int count = 0;
        
        for (auto& att : anEntity->getAttributes()) {
            ++count;
            //field name
            output << "| " << std::setw(20) << std::left << att.getName();
            
            //data type
            output << "| " << std::setw(13) << std::left
                << typeToString(att.getType(), att.getLength());

            //nullable
            output << "| " << std::setw(5) << std::left;
            if (att.isNullable())
                output << "YES";
            else
                output << "NO";

            //primary key
            output << "| " << std::setw(4) << std::left;
            if (att.isPrimaryKey())
                output << "YES";
            else
                output << " ";

            //default value
            output << "| " << std::setw(8) << std::left;
            if (att.hasDefaultValue())
                output << att.getDefaultValue();
            else
                output << "NULL";

            //extra information
            output << "| " << std::setw(28) << std::left;
            std::string temp = "";
            if (att.isAutoIncrement())
                temp += "auto_increment ";
            if (att.isPrimaryKey())
                temp += "primary key ";
            output << temp << "|\n";
        }
        output << "+---------------------+--------------+------+-----+---------+-----------------------------+\n"
            << count << " rows in set ";

        return true;
    }

    static void setSeperationBar(std::ostream& anOutput, std::unordered_map<std::string, size_t>& aMaxSize,
        std::shared_ptr<Query>& aQuery, const size_t aRowWidth) {
        bool selectAll = true;
        StringList selects;

        //get select fields
        if (!aQuery->selectAll()) {
            selectAll = false;
            selects = aQuery->getSelects();
        }
        else {
            for (auto& att : aQuery->getFrom()->getAttributes())
                selects.push_back(att.getName());
        }

        anOutput << "+";

        //id is the first column by default
        if (selectAll || std::find(selects.begin(), selects.end(), "id") != selects.end()) {
            for (int i = 0; i <= aMaxSize["id"]; ++i)
                anOutput << '-';
            anOutput << "+";
        }

        //iterate selected attributes
        for (auto& cur : selects) {
            if (cur == "id")
                continue;
            if (selectAll || std::find(selects.begin(), selects.end(), cur) != selects.end()) {
                for (int i = 0; i <= aMaxSize[cur]; ++i)
                    anOutput << '-';

                anOutput << '+';
            }
        }

        anOutput << '\n';
    }

    static void setFieldBar(std::ostream& anOutput, std::unordered_map<std::string, size_t>& aMaxSize,
        std::shared_ptr<Query>& aQuery, const size_t kRowWidth) {
        bool selectAll = true;
        StringList selects;

        //get select fields
        if (!aQuery->selectAll()) {
            selectAll = false;
            selects = aQuery->getSelects();
        }
        else {
            for (auto& att : aQuery->getFrom()->getAttributes())
                selects.push_back(att.getName());
        }

        //id is the first column by default
        if (selectAll || std::find(selects.begin(), selects.end(), "id") != selects.end()) {
            anOutput << "| " << std::setw(aMaxSize["id"]) << std::left << "id";
        }

        //iterate and show all attributes
        for (auto& cur : selects) {
            if (cur == "id")
                continue;
            if (selectAll || std::find(selects.begin(), selects.end(), cur) != selects.end())
                anOutput << "| " << std::setw(aMaxSize[cur]) << std::left << cur;
        }

        anOutput << "|\n";
    }

    static void reorderRow(std::shared_ptr<Query>& aQuery, RowCollection& aCollection, 
        OrderedRow& anOrderedRow) {
        size_t maxSize = 0;

        std::string orderBy = "id";
        auto ascend = aQuery->getAscend();
        if (aQuery->getOrderBy().size()) {
            orderBy = aQuery->getOrderBy()[0];
        }        

        for (auto& row : aCollection) {
            std::stringstream ss;
            ss << row->getValue(orderBy);
            std::string key = ss.str();
            anOrderedRow[key].push_back(std::move(row));
        }
    }
    
    static std::unordered_map<std::string, size_t> getAttributeMaxSize(OrderedRow& aRows, const size_t kRowWidth) {
        std::unordered_map<std::string, size_t> res;

        for (auto& field : aRows) {
            for (auto& row : field.second) {
                for (auto& cur : row->getData()) {
                    std::string* str = std::get_if<std::string>(&cur.second);
                    res[cur.first] = std::max(res[cur.first], cur.first.size() + 1);
                    if (str)
                        res[cur.first] = std::max(res[cur.first], str->size() + 1);
                    else
                        res[cur.first] = std::max(kRowWidth, res[cur.first]);
                }
            }
        }

        return res;
    }

    static void showData(std::ostream& anOutput, std::shared_ptr<Query>& aQuery, OrderedRow& aRows,
        std::unordered_map<std::string, size_t>& aMaxSize, 
        const size_t kRowWidth) {
        /* data row, will look like
        *  | 0    | chandhini      | grandhi        |
        *  | 1    | rick           | gessner        |
        *  | 2    | savya          |                | */       

        bool selectAll = true;
        StringList selects;

        //get select fields
        if (!aQuery->selectAll()) {
            selectAll = false;
            selects = aQuery->getSelects();
        }
        else {
            for (auto& att : aQuery->getFrom()->getAttributes())
                selects.push_back(att.getName());
        }

        //output data
        for (auto& field : aRows) {
            for (auto& row : field.second) {
                KeyValues data = row->getData();

                //id field is the first column by default
                if (selectAll || std::find(selects.begin(), selects.end(), "id") != selects.end())
                    anOutput << "| " << std::setw(aMaxSize["id"]) << std::left << data["id"];

                for (auto& cur : selects) {
                    //id field is already taken care
                    if (cur == "id") {
                        continue;
                    }

                    if (selectAll || std::find(selects.begin(), selects.end(), cur) != selects.end()) {
                        anOutput << "| " << std::setw(aMaxSize[cur]) << std::left;
                        if (data[cur].index() == 0) { //is a bool
                            auto val = std::get_if<bool>(&data[cur]);

                            if (*val == true)
                                anOutput << "true";
                            else
                                anOutput << "false";
                        }
                        else { //is int, double or string
                            anOutput << data[cur];
                        }
                    }
                }
                anOutput << "|\n";
            }
        }
    }

    bool QueryView::showQuery(std::shared_ptr<Query>& aQuery, RowCollection& aCollection) {
        const size_t kRowWidth = 9;

        if (!aQuery)
            return false;

        //reorder data        
        OrderedRow orderedRow;
        reorderRow(aQuery, aCollection, orderedRow);

        auto maxSize = getAttributeMaxSize(orderedRow, kRowWidth);

        setSeperationBar(output, maxSize, aQuery, kRowWidth);                
        setFieldBar(output, maxSize, aQuery, kRowWidth);        
        setSeperationBar(output, maxSize, aQuery, kRowWidth);
        showData(output, aQuery, orderedRow, maxSize, kRowWidth);
        setSeperationBar(output, maxSize, aQuery, kRowWidth);

        output << aCollection.size() << " rows in set ";

        return true;
    }

    void IndexView::showPairs(IndexPairs& anIndexes, bool all) {
        output << "+-----------------+-----------------+\n";

        if (all)
            output << "| table           | field(s)        |\n";
        else
            output << "| key             | block#          |\n";

        output << "+-----------------+-----------------+\n";
        

        for (auto& cur : anIndexes) {
            output 
                << "| " << std::setw(16) << std::left << cur.first
                << "| " << std::setw(16) << std::left << cur.second << "|\n"
                << "+-----------------+-----------------+\n";
        }

        output << anIndexes.size() << " rows in set ";
    }

    bool DebugView::debugDump(std::unique_ptr<std::vector<BlockHeader>>& aHeaders) {
        static std::unordered_map< BlockType, std::string> blockTypeToString {
            {BlockType::meta_block, "meta"},
            {BlockType::data_block, "data"},
            {BlockType::entity_block, "entity"},
            {BlockType::free_block, "free"},
            {BlockType::index_block, "index"},
            {BlockType::unknown_block, "unknown"}
        };

        if (!aHeaders)
            return false;

        output
            << "+----------------+-----------+---------------+\n"
            << "| Type           | Id        | Extra         |\n"
            << "+----------------+-----------+---------------+\n";

        for (auto& cur : *aHeaders) {
            output << "| " << std::setw(15) << std::left << blockTypeToString[BlockType{ cur.type }]
                << "| " << std::setw(10) << std::left;

            //id and extra
            switch (cur.type) {
            case((int)BlockType::meta_block):
                output << "0"
                    << "| " << std::setw(14) << std::left << "";
                break;
            case((int)BlockType::data_block):
                output << cur.id
                    << "| " << std::setw(14) << std::left << cur.refId;
                break;
            case((int)BlockType::entity_block):
                output << cur.refId
                    << "| " << std::setw(14) << std::left << "";
                break;
            case((int)BlockType::free_block):
                output << "0"
                    << "| " << std::setw(14) << std::left << "";
                break;
            case((int)BlockType::index_block):
                output << cur.refId
                    << "| " << std::setw(14) << std::left << "";
                break;

            default:
                output << "0"
                    << "| " << std::setw(14) << std::left << "";
                break;
            }

            output << "|\n" << "+----------------+-----------+---------------+\n";
        }

        output << aHeaders->size() << " rows in set ";

        return true;
    }

}