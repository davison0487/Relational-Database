//
//  Row.cpp
//  Assignment4
//
//  Created by rick gessner on 4/19/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#include "Row.hpp"
#include "Database.hpp"


namespace ECE141 {

    template<typename T>
    static char vtype(const T& aVar) {
        static char theTypes[] = { 'b','i','d','s' };
        return theTypes[aVar.index()];
    }

    static std::ostream& operator<< (std::ostream& out, const Value& aValue) {
        std::visit([theType = vtype(aValue), &out](auto const& aValue)
        { out << theType << ' ' << '\"' << aValue << '\"'; }, aValue);
        return out;
    }

    int Row::getID() {
        int id = -1;

        std::visit([&](const Value& aValue) {
            if (aValue.index() == 1)
                id = std::get<int>(aValue);
            }, data["id"]);

        return id;
    }

    Value Row::getValue(std::string aKey) {
        if(data.count(aKey))
            return data[aKey];

        return 0;
    }

    KeyValues& Row::getData() {
        return data;
    }

    uint32_t Row::getBlockNum() {
        return blockNumber;
    }

    void Row::setId(int anID) {
        data["id"] = anID;
    }

    bool Row::update(KeyValues aNewData) {
        for (auto& cur : aNewData) {
            if (data.count(cur.first))
                data[cur.first] = cur.second;
            else
                return false;
        }

        return true;
    }

    void Row::addData(std::string aName, Value aValue) {
        data[aName] = aValue;
    }

    void Row::dropData(std::string aName) {
        data.erase(aName);
    }

	StatusResult Row::encode(std::ostream& aWriter) {
        aWriter << blockNumber << ' ';
        for (auto& cur : data) {
            //the name
            aWriter << cur.first << ' ';
            //the data
            aWriter << cur.second << ' ';
        }
        
        return StatusResult{ noError };
	}

    namespace DecodeHelper {
        std::string readValue(std::istream& aReader) {
            std::string res = "";
            aReader.get(); //skip space
            aReader.get(); //skip quote

            while (!aReader.eof()) {
                char ch = aReader.get();
                
                if (ch == '\"')
                    break;

                res.push_back(ch);
            }

            return res;
        }
    }

    //read string into boolean
    static bool stob(std::string aStr) {
        if (stoi(aStr))
            return true;
        else
            return false;
    }

	StatusResult Row::decode(std::istream& aReader) {
        std::string temp;

        aReader >> temp;
        blockNumber = std::stoul(temp);

        while (aReader >> temp) {
            //key
            std::string key = temp;

            //type
            aReader >> temp;
            char theType = temp[0];

            //value
            temp = DecodeHelper::readValue(aReader);
            switch (theType) {
            case 'b':
                data[key] = stob(temp);
                break;
            case 'i':
                data[key] = std::stoi(temp);
                break;
            case 'd':
                data[key] = std::stod(temp);
                break;
            case 's':
                data[key] = temp;
                break;
            }
        }

        return StatusResult{ noError };
	}

}
