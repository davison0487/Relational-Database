//
//  Entity.cpp
//
//  Created by rick gessner on 4/03/21.
//  Copyright © 2021 rick gessner. All rights reserved.
//

#include <sstream>
#include "Entity.hpp"


namespace ECE141 {
  const int gMultiplier = 37;

  //hash given string to numeric quantity...
  uint32_t Entity::hashString(const char *str) {
    uint32_t h{0};
    unsigned char *p;
    for (p = (unsigned char*)str; *p != '\0'; p++)
      h = gMultiplier * h + *p;
    return h;
  }

  uint32_t Entity::hashString(const std::string &aStr) {
    return hashString(aStr.c_str());
  }
  
  Entity::Entity(std::string aName, const AttributeList& anAttList)
      : name(aName), attributes(anAttList), increment(1) {}

  Entity::Entity(const Entity& aCopy)
      : name(aCopy.name), attributes(aCopy.attributes), increment(aCopy.increment) {}

  Entity& Entity::operator=(const Entity* aCopy) {
      this->attributes = aCopy->attributes;
      this->name = aCopy->name;
      return *this;
  }

  Entity& Entity::operator=(const Entity& aCopy) {
      this->attributes = aCopy.attributes;
      this->name = aCopy.name;
      return *this;
  }

  Attribute* Entity::getPrimaryKey() {
      for (auto& att : attributes) {
          if (att.isPrimaryKey()) {
              return &att;
          }
      }
      return nullptr;
  }

  StatusResult Entity::addAttribute(Attribute& anAtt) {
      attributes.push_back(anAtt);
      return StatusResult{ Errors::noError };
  }

  StatusResult Entity::dropAttribute(Attribute& anAtt) {
      for (int i = 0; i < attributes.size(); ++i) {
          if (attributes[i].getName() == anAtt.getName()) {
              attributes.erase(attributes.begin() + i);
              return StatusResult{ Errors::noError };
          }
      }

      return StatusResult{ Errors::unknownAttribute };
  }

  StatusResult Entity::encode(std::ostream &aWriter) {
      aWriter << name << ' ' << increment << ' ';

      //encode attributes
      for (auto attribute : attributes) {
          attribute.encode(aWriter);
      }

      aWriter << '#' << ' '; //an eof flag

      return StatusResult{noError};
  }

  StatusResult Entity::decode(std::istream &aReader) {
      aReader >> name;

      //increment
      std::string temp;
      aReader >> temp;
      increment = stoul(temp);

      aReader.get(); //eat the space

      //decode attributes
      while (aReader.peek() != '#') {
          Attribute att;
          att.decode(aReader);
          attributes.push_back(att);
      }

      return StatusResult{noError};
  }

}
