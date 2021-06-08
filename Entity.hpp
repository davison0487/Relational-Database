//
//  Entity.hpp
//
//  Created by rick gessner on 4/03/21.
//  Copyright © 2021 rick gessner. All rights reserved.
//


#ifndef Entity_hpp
#define Entity_hpp

#include <stdio.h>
#include <vector>
#include <optional>
#include <memory>
#include <string>

#include "Attribute.hpp"
#include "BasicTypes.hpp"
#include "Errors.hpp"
#include "Storage.hpp"

namespace ECE141 {
  
  using AttributeOpt = std::optional<Attribute>;
  using AttributeList = std::vector<Attribute>;

  //------------------------------------------------

  class Entity : public Storable  {
  public:

    static uint32_t       hashString(const char *str);
    static uint32_t       hashString(const std::string &aStr);

    Entity(std::string aName, const AttributeList& anAttList);
    Entity(const Entity& aCopy);
    ~Entity() {};

    Entity& operator=(const Entity* aCopy);
    Entity& operator=(const Entity& aCopy);
        
    uint32_t hashName() { return hashString(name); }
        
    std::string getName() { return name; }

    uint32_t getIncrement() { return increment++; }   

    //get primary key attribute
    Attribute* getPrimaryKey();

    //get certain attribute
    Attribute* getAttribute(std::string aName) {
        for (auto& att : attributes) {
            if (att.getName() == aName)
                return &att;
        }
        return nullptr;
    }

    //get all attributes
    AttributeList getAttributes() { return attributes; }

    StatusResult addAttribute(Attribute& anAtt);
    StatusResult dropAttribute(Attribute& anAtt);
    
    /*----------------Storable----------------*/
    StatusResult          encode(std::ostream &aWriter) override;
    StatusResult          decode(std::istream &aReader) override;
    /*----------------Storable----------------*/

  protected:

    std::string   name;
    AttributeList attributes;
    uint32_t      increment;
  };
  
}
#endif /* Entity_hpp */
