//
//  Query.hpp
//  Assignment5
//
//  Created by rick gessner on 4/26/21.
//

#ifndef Query_hpp
#define Query_hpp

#include <stdio.h>
#include <string>
#include <optional>
#include <vector>
#include "Attribute.hpp"
#include "Row.hpp"
#include "Entity.hpp"
#include "Tokenizer.hpp"
#include "Filters.hpp"

namespace ECE141 {

  class Query  {
  public:
    
    Query();
    Query(const Query& aCopy);
    ~Query();

    Query& operator=(const Query& aCopy);
    
    //access data
    std::string              getEntityName() const;
    bool                     selectAll() const;
    StringList               getSelects() const;
    Entity*                  getFrom() const;
    std::vector<std::string> getOrderBy() const;
    std::vector<bool>        getAscend() const;
    int                      getOffset() const;
    int                      getLimit() const;

    //set data
    Query& setEntityName(std::string aName);
    Query& setSelectAll(bool aState);
    Query& setSelect(const StringList &aFields);
    Query& setSelect(std::string aField);
    Query& setFrom(Entity *anEntity); 
    Query& setOrderBy(std::string aField, bool anAscending = true);
    Query& setOffset(int anOffset);    
    Query& setLimit(int aLimit);
    void setLogic(Operators anOp);

    StatusResult parseFilters(Tokenizer& aTokenizer);
        
    bool matches(KeyValues& aList);

    /*
    DBQuery& orderBy(const std::string &aField, bool ascending=false);

    //how will you handle where clause?
    //how will you handle group by?

    DBQuery& run(Rows &aRows);
    */
    
  protected:
    std::string entityName;
    Entity*    _from;
    StringList fields;

    //used by QueryView::showQuery()
    std::vector<std::string> orderBy;
    std::vector<bool>        ascend;

    //used by Database::selectRows()
    bool       all;
    int        offset;
    int        limit;
    Filters    filters;
    
  };

}

#endif /* Query_hpp */
