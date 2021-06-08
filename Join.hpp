//
//  Join.hpp
//  RGAssignment8
//
//  Created by rick gessner on 5/05/21.
//

#ifndef Join_h
#define Join_h

#include <string>
#include <vector>
#include "BasicTypes.hpp"
#include "Errors.hpp"
#include "keywords.hpp"
#include "Filters.hpp"


namespace ECE141 {

    struct TableField {
        TableField(const TableField& aCopy) {
            tableName = aCopy.tableName;
            fieldName = aCopy.fieldName;
        }

        TableField(const std::string& aTable = "", const std::string& aField = "")
            : tableName(aTable), fieldName(aField) {}

        TableField& operator=(const TableField& aCopy) {
            tableName = aCopy.tableName;
            fieldName = aCopy.fieldName;
            return *this;
        }

        std::string tableName;
        std::string fieldName;
    };

    struct Join {
        Join(const std::string& aTable, Keywords aType, const TableField& aLHS, const TableField& aRHS)
            : table(aTable), joinType(aType), onLeft(aLHS), onRight(aRHS) {}

        Keywords    joinType;
        std::string table;
        TableField  onLeft;
        TableField  onRight;
    };

  using JoinList = std::vector<Join>;

}

#endif /* Join_h */
