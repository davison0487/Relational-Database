//
//  Row.hpp
//  Assignment4
//
//  Created by rick gessner on 4/19/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef Row_hpp
#define Row_hpp

#include <stdio.h>
#include <string>
#include <utility>
#include <variant>
#include <vector>
#include <memory>
#include "Storage.hpp"
#include "Attribute.hpp"


class Database;

namespace ECE141 {

  class Row : public Storable {
  public:

      Row() : blockNumber(0) {}

      Row(KeyValues aData, uint32_t aBlockNumber)
          : data(aData), blockNumber(aBlockNumber) {}

      Row(const Row& aCopy)
          : data(aCopy.data), blockNumber(aCopy.blockNumber) {}

      ~Row() {}

      Row& operator=(const Row& aCopy) {
          data = aCopy.data;
          blockNumber = aCopy.blockNumber;
          return *this;
      }

      int getID();

      Value getValue(std::string aKey);

      KeyValues& getData();

      uint32_t getBlockNum();

      void setId(int anID);
      
      bool update(KeyValues aNewData);

      void addData(std::string aName, Value aValue);

      void dropData(std::string aName);

      /*----------------Storable----------------*/
      StatusResult encode(std::ostream &aWriter) override;
      StatusResult decode(std::istream &aReader) override;
      void         initBlock(Block &aBlock);
      /*----------------Storable----------------*/

  protected:
      KeyValues           data;
      uint32_t            blockNumber;

  };

  //-------------------------------------------

  using RowCollection = std::vector<std::unique_ptr<Row> >;

}

#endif /* Row_hpp */
