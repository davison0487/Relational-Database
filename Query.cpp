//
//  Query.cpp
//  Assignment5
//
//  Created by rick gessner on 4/26/21.
//

#include <limits>
#include "Query.hpp"

namespace ECE141 {

    Query::Query() : _from(nullptr), all(false), offset(0), limit(std::numeric_limits<int>::max()) {}

    Query::Query(const Query& aCopy) {
        entityName = aCopy.entityName;
        _from = aCopy._from;
        fields = aCopy.fields;
        all = aCopy.all;
        offset = aCopy.offset;
        limit = aCopy.limit;
        orderBy = aCopy.orderBy;
    }

    Query::~Query() {}

    Query& Query::operator=(const Query& aCopy) {
        entityName = aCopy.entityName;
        _from = aCopy._from;
        fields = aCopy.fields;
        all = aCopy.all;
        offset = aCopy.offset;
        limit = aCopy.limit;
        orderBy = aCopy.orderBy;
        ascend = aCopy.ascend;
        return *this;
    }

    std::string Query::getEntityName() const {
        return entityName;
    }

    bool Query::selectAll() const {
        return all;
    }

    StringList Query::getSelects() const {
        return fields;
    }

    Entity* Query::getFrom() const {
        return _from;
    }

    std::vector<std::string> Query::getOrderBy() const {
        return orderBy;
    }
    
    std::vector<bool> Query::getAscend() const {
        return ascend;
    }

    int Query::getOffset() const {
        return offset;
    }

    int Query::getLimit() const {
        return limit;
    }

    Query& Query::setEntityName(std::string aName) {
        entityName = aName;
        return *this;
    }

    Query& Query::setSelectAll(bool aState) {
        all = aState;
        return *this;
    }

    Query& Query::setSelect(const StringList& aFields) {
        all = false;
        fields = aFields;
        return *this;
    }

    Query& Query::setSelect(std::string aField) {
        all = false;
        fields.push_back(aField);
        return *this;
    }

    Query& Query::setFrom(Entity* anEntity) {
        _from = anEntity;
        return *this;
    }
    
    Query& Query::setOrderBy(std::string aField, bool anAscending) {
        orderBy.push_back(aField);
        ascend.push_back(anAscending);
        return *this;
    }

    Query& Query::setOffset(int anOffset) {
        offset = anOffset;
        return *this;
    }

    Query& Query::setLimit(int aLimit) {
        limit = aLimit;
        return *this;
    }

    void Query::setLogic(Operators anOp) {
        filters.setLogic(anOp);
    }

    StatusResult Query::parseFilters(Tokenizer& aTokenizer) {
        return filters.parse(aTokenizer, *_from);
    }

    bool Query::matches(KeyValues& aList) {
        return filters.matches(aList);
    }
}
