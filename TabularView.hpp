
//
//  RecordsView.hpp
//
//  Created by rick gessner on 4/26/20.
//  Copyright © 2021 rick gessner. All rights reserved.
//

#ifndef TabularView_h
#define TabularView_h

#include <iostream>
#include <sstream>
#include <iomanip>
#include <map>
#include <vector>
#include <string>
#include "View.hpp"
#include "Row.hpp"

//IGNORE THIS CLASS if you already have a class that does this...

namespace ECE141 {

  // USE: general tabular view (with columns)
  class TabularView : public View {
  public:
    
    //STUDENT: Add OCF methods...
        
    bool show(std::ostream &anOutput) {
      return true;
    }
    
  protected:
    //what data members do you require?
  };

}

#endif /* TabularView_h */
