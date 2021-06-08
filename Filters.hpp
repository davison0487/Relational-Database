//
//  Filters.hpp
//  RGAssignment5
//
//  Created by rick gessner on 4/4/21.
//  Copyright © 2021 rick gessner. All rights reserved.
//

#ifndef Filters_h
#define Filters_h

#include <stdio.h>
#include <vector>
#include <memory>
#include <string>
#include "Errors.hpp"
#include "Tokenizer.hpp"
#include "BasicTypes.hpp"

namespace ECE141 {
  
  class Row;
  class Entity;
  
  struct Operand {
    Operand() {}
    Operand(std::string &aName, TokenType aType, Value &aValue, size_t anId=0)
      : ttype(aType), dtype(DataTypes::varchar_type), name(aName),
        value(aValue), entityId(anId) {}
    
    TokenType   ttype; //is it a field, or const (#, string)...
    DataTypes   dtype;
    std::string name;  //attr name
    Value       value;
    size_t      entityId;
  };
  
  //---------------------------------------------------

  enum class Logical { no_op = 0, or_op, and_op, not_op };

  struct Expression {
    Operand     lhs;  //id
    Operand     rhs;  //usually a constant; maybe a field...
    Operators   op;   //=     //users.id=books.author_id
    Logical     logic; //and, or, not...
    
    Expression()
        : lhs(), rhs(), op(Operators::equal_op), logic(Logical::no_op) {}

    Expression(Operand &aLHSOperand, Operators anOp,
               Operand &aRHSOperand)
      : lhs(aLHSOperand), rhs(aRHSOperand),
        op(anOp), logic(Logical::no_op) {}
    
    bool operator()(KeyValues &aList);

    void addLogic(Operators anOp);

    Logical getLogic();
  };
  
  using Expressions = std::vector<std::unique_ptr<Expression> >;

  //---------------------------------------------------

  class Filters {
  public:
    
    Filters();
    Filters(const Filters &aFilters);
    ~Filters();
    
    size_t        getCount() const {return expressions.size();}
    bool          matches(KeyValues &aList) const;
    Filters&      add(Expression *anExpression);

    Filters&      setLogic(Operators anOp);
        
    StatusResult  parse(Tokenizer &aTokenizer, Entity &anEntity);
    
  protected:
    Expressions   expressions;
  };
 
}

#endif /* Filters_h */
