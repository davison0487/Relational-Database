//
//  Attribute.hpp
//
//  Created by rick gessner on 4/02/21.
//  Copyright © 2021 rick gessner. All rights reserved.
//

#ifndef Attribute_hpp
#define Attribute_hpp

#include <stdio.h>
#include <string>
#include <variant>
#include <optional>
#include "Storage.hpp"
#include "keywords.hpp"
#include "BasicTypes.hpp"

namespace ECE141 {

  class Attribute : public Storable {
  protected:
    std::string name;
    DataTypes   type;
    int         length;
    bool        auto_increment;
    bool        primary_key;
    bool        nullable;
    bool        hasDefault;
    Value       default_value;
    
  public:
      Attribute();

      Attribute(std::string aName, DataTypes aType,
          int aLength = 0,
          bool aNullable = true,
          bool anAutoIncrement = false,
          bool aPrimaryKey = false,
          bool aHasDefault = false,
          Value aDefault = 0);

      Attribute(const Attribute& aCopy);

      ~Attribute();
      
      Attribute& operator=(const Attribute& aCopy);

      //set data
      void setName(std::string aName) { name = aName; }
      void setType(DataTypes aType) { type = aType; }
      void setLength(int aLength) { length = aLength; }
      void setAutoIncrement(bool aAuto_increment) { auto_increment = aAuto_increment; }
      void setPrimaryKey(bool aPrimaryKey) { primary_key = aPrimaryKey; }
      void setNullable(bool aNullable) { nullable = aNullable; }
      void setHasDefault(bool aHasDefault) { hasDefault = aHasDefault; }
      void setDefaultValue(Value aDefault) { default_value = aDefault; }
    
      //access data
      std::string getName()         { return name; }
      DataTypes   getType()         { return type; }
      int         getLength()       { return length; }
      bool        isAutoIncrement() { return auto_increment; }
      bool        isPrimaryKey()    { return primary_key; }
      bool        isNullable()      { return nullable; }
      bool        hasDefaultValue() { return hasDefault; }
      Value       getDefaultValue() { return default_value; }

      /*----------------Storable----------------*/
      StatusResult encode(std::ostream& aWriter) override;
      StatusResult decode(std::istream& aReader) override;
      /*----------------Storable----------------*/
    
  };
  
}


#endif /* Attribute_hpp */
