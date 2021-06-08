//
//  Attribute.hpp
//
//  Created by rick gessner on 4/02/21.
//  Copyright © 2021 rick gessner. All rights reserved.
//

#include <iostream>
#include "Attribute.hpp"

namespace ECE141 {

    Attribute::Attribute()
        :name(""), type(DataTypes{ 'N' }), length(0), nullable(false),
        auto_increment(false), primary_key(false), hasDefault(false), default_value(0) {}

    Attribute::Attribute(std::string aName, DataTypes aType, int aLength, bool aNullable, 
        bool anAutoIncrement, bool aPrimaryKey, bool aHasDefault, Value aDefault)
        : name(aName), type(aType), length(aLength), nullable(aNullable),
        auto_increment(anAutoIncrement), primary_key(aPrimaryKey), 
        hasDefault(aHasDefault), default_value(aDefault) {}

    Attribute::Attribute(const Attribute& aCopy)
        : name(aCopy.name), type(aCopy.type), length(aCopy.length), nullable(aCopy.nullable),
        auto_increment(aCopy.auto_increment), primary_key(aCopy.primary_key),
        hasDefault(aCopy.hasDefault), default_value(aCopy.default_value) {}

    Attribute& Attribute::operator=(const Attribute& aCopy) {
        name           = aCopy.name;
        type           = aCopy.type;
        length         = aCopy.length;
        auto_increment = aCopy.auto_increment;
        primary_key    = aCopy.primary_key;
        hasDefault     = aCopy.hasDefault;
        default_value  = aCopy.default_value;

        return *this;
    }

    Attribute::~Attribute() {}

    template<typename T>
    static char vtype(const T& aVar) {
        static char theTypes[] = { 'b','i','d','s' };
        return theTypes[aVar.index()];
    }

    static std::ostream& operator<< (std::ostream& out, const Value& aValue) {
        std::visit([theType = vtype(aValue), &out](auto const& aValue)
        { out << theType << ' ' << aValue; }, aValue);
        return out;
    }

    StatusResult Attribute::encode(std::ostream& aWriter) {
        aWriter << name << ' '
            << char(type) << ' '
            << length << ' '
            << auto_increment << ' '
            << primary_key << ' '
            << nullable << ' '
            << hasDefault << ' '
            << default_value << ' ';

        return StatusResult{};
    }
    
    //string to boolean
    static bool stob(std::string aStr) {
        if (stoi(aStr))
            return true;
        else
            return false;
    }

    StatusResult Attribute::decode(std::istream& aReader) {
        std::string temp;
        
        //name
        aReader >> name;

        //type
        aReader >> temp;
        type = DataTypes{ temp[0] };

        //length
        aReader >> temp;
        length = std::stoi(temp);

        //auto_increment
        aReader >> temp;
        auto_increment = stob(temp);

        //primary_key
        aReader >> temp;
        primary_key = stob(temp);

        //nullable
        aReader >> temp;
        nullable = stob(temp);

        //has default
        aReader >> temp;
        hasDefault = stob(temp);

        //default_value
        aReader >> temp;
        char theType = temp[0];
        aReader >> temp;
        switch (theType) {
        case 'b':
            default_value = stob(temp);
            break;
        case 'i':
            default_value = std::stoi(temp);
            break;
        case 'd':
            default_value = std::stod(temp);
            break;
        case 's':
            default_value = temp;
            break;
        }

        //eat the space
        aReader.get();

        return StatusResult{};
    }

}
