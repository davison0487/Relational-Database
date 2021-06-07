//
//  main.cpp
//  Database2
//
//  Created by rick gessner on 3/17/19.
//  Copyright © 2019 rick gessner. All rights reserved.
//

#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <cmath>
#include <functional>
#include <variant>
#include <ctime>
#include <type_traits>
#include "TestManually.hpp"
#include "TestAutomatic.hpp"
#include "LFUCache.hpp"

int main(int argc, const char * argv[]) {
  
  srand(static_cast<uint32_t>(time(0)));
  
  if(argc>1) {
    ECE141::TestAutomatic theTests;
    std::map<std::string, std::function<bool()> > theCalls {
      {"Alter",  [&](){return theTests.doAlterTest();}},
      {"App",    [&](){return theTests.doAppTest();}},
      {"Cache",  [&](){return theTests.doCacheTest();}},
      {"Compile",[&](){return theTests.doCompileTest();}},
      {"DB",     [&](){return theTests.doDBTest();}},
      {"Delete", [&](){return theTests.doDeleteTest();}},
      {"Drop",   [&](){return theTests.doDropTest();}},
      {"Index",  [&](){return theTests.doIndexTest();}},
      {"Insert", [&](){return theTests.doInsertTest();}},
      {"Join",   [&](){return theTests.doJoinTest();}},
      {"Select", [&](){return theTests.doSelectTest();}},
      {"Tables", [&](){return theTests.doTablesTest();}},
      {"Update", [&](){return theTests.doUpdateTest();}},
    };
    
    std::string theCmd(argv[1]);
    if(theCalls.count(theCmd)) {
      bool theResult = theCalls[theCmd]();
      const char* theStatus[]={"FAIL","PASS"};
      std::cout << theCmd << " test " << theStatus[theResult] << "\n";
    }
    else std::cout << "Unknown test\n";
  }
  else {
    doManualTesting();
  }
  return 0;
}
