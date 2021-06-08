//
//  View.hpp
//  RGAssignment2
//
//  Created by rick gessner on 2/27/21.
//

#ifndef View_h
#define View_h

#include <string>
#include <iostream>
#include <vector>
#include <memory>
#include "BasicTypes.hpp"
#include "BlockIO.hpp"
#include "Config.hpp"
#include "Row.hpp"
#include "Entity.hpp"
#include "Query.hpp"


namespace ECE141 {
    
  using ShowResult = std::function<void(std::ostream& anOutput)>;
  
  class View {
  public:
      View(std::ostream& anOutput) : output(anOutput) {}
      
      virtual         ~View() {}

      //a basic show command that accepts a callable
      bool show(ShowResult aShow);

  protected:
      std::ostream& output;
  };
  
  class DBView : public View {
  public:
      DBView(std::ostream& anOutput, std::string aPath = Config::getStoragePath(), std::string anExtension = Config::getDBExtension())
          : View(anOutput), path(aPath), extension(anExtension) {}

      ~DBView() {}

      //show all databases
      void showDBs();

  private:
      std::string path;
      std::string extension;
  };

  class TableView : public View {
  public:
      TableView(std::ostream& anOutput) : View(anOutput) {}

      ~TableView() {}
      
      //show all tables
      void showTables(std::string aName, std::vector<std::string> aTables);
  };

  class DescribeView : public View {
  public:
      DescribeView(std::ostream& anOutput) : View(anOutput) {}

      ~DescribeView() {}

      //describe a given table
      bool describeTable(Entity* anEntity);
  };

  class QueryView : public View {
  public:
      QueryView(std::ostream& anOutput) : View(anOutput) {}

      ~QueryView() {}

      //show rows with given query
      bool showQuery(std::shared_ptr<Query>& aQuery, RowCollection& aCollection);
  };

  class IndexView : public View {
  public:
      IndexView(std::ostream& anOutput) : View(anOutput) {}

      ~IndexView() {}

      void showPairs(IndexPairs& anIndexes, bool all = false);
  };
  
  class DebugView : public View {
  public:
      DebugView(std::ostream& anOutput) : View(anOutput) {}

      ~DebugView() {}

      //for debugging
      bool debugDump(std::unique_ptr<std::vector<BlockHeader>>& aHeaders);
  };
}

#endif /* View_h */
