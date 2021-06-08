//
//  TestAutomatic.hpp
//  RGAssignement2
//
//  Created by rick gessner on 2/26/21.
//

#ifndef TestAutomatic_h
#define TestAutomatic_h

#include "Application.hpp"
#include "Config.hpp"
#include <sstream>
#include "Errors.hpp"
#include "Faked.hpp"
#include <sstream>
#include <algorithm>
#include <map>
#include <vector>
#include <cctype>
#include <cstdlib>
#include <ctime>

namespace ECE141 {

  using CountList = std::vector<int>;

  void showErrors(ECE141::StatusResult &aResult, std::ostream &anOutput) {
    
    static std::map<ECE141::Errors, std::string> theMessages = {
      {ECE141::illegalIdentifier, "Illegal identifier"},
      {ECE141::unknownIdentifier, "Unknown identifier"},
      {ECE141::databaseExists, "Database exists"},
      {ECE141::tableExists, "Table Exists"},
      {ECE141::syntaxError, "Syntax Error"},
      {ECE141::unknownCommand, "Unknown command"},
      {ECE141::unknownDatabase,"Unknown database"},
      {ECE141::unknownTable,   "Unknown table"},
      {ECE141::unknownError,   "Unknown error"}
    };

    std::string theMessage="Unknown Error";
    if(theMessages.count(aResult.error)) {
      theMessage=theMessages[aResult.error];
    }
    anOutput << "Error (" << aResult.error << ") "
      << theMessage << "\n";
  }

  //------------------------------

  class TestAutomatic {
  protected:
    std::stringstream output;
  public:
    
    ~TestAutomatic() {
      std::cout << "Test Version 1.9.5\n";
      std::cout << "---- output ---------------\n";
      std::cout << output.str() << "\n";
    }

    bool doCompileTest() {return true;}

    bool doScriptTest(std::istream &anInput, std::ostream &anOutput) {
      ECE141::Application   theApp(anOutput);
      ECE141::StatusResult  theResult{};
      std::string theCommand;
      
      while(theResult && anInput) {
        std::getline (anInput, theCommand);
        std::stringstream theStream(theCommand);
        anOutput << theCommand << "\n";
        theResult=theApp.handleInput(theStream);
        if(theResult==ECE141::userTerminated) {
          theResult.error=Errors::noError;
          break;
        }
        else if(!theResult) {
          showErrors(theResult, anOutput);
        }
      }
      return theResult;
    }
    
    //----------------------------------------------

    bool doAppTest() {

      std::string theInput("version;\n help;\n quit;");
      std::stringstream theStream(theInput);

      bool theResult=doScriptTest(theStream, output);
      if(theResult) {
        const char* theLines[]={
          "Version 0.9.", "Help system ready.","DB::141 is shutting down."
        };
        
        std::string temp=output.str();
        //std::cout << temp << "\n";
        std::stringstream theStream(temp);
        std::string theLine;

        for(auto *theNext : theLines) {
          std::getline(theStream,theLine); //skip command...
          std::getline(theStream,theLine);
          if(theLine!=theNext) return false;
        }
        return true;
      }
      
      return false;
    }

    enum states {wasUnknown, wasAlter, wasCreate, wasDescribe,
        wasDelete, wasDrop, wasDump, wasInsert, wasSelect,
        wasShow, wasUpdate, wasUse};
    
    //convert from string (word) to states enum...
    states getTerm(const std::string &aWord) {
      static std::map<std::string, states> gTerms = {
        {"alter",states::wasAlter},
        {"create",states::wasCreate},
        {"delete",states::wasDelete},
        {"describe",states::wasDescribe},
        {"drop",states::wasDrop},
        {"dump",states::wasDump},
        {"insert",states::wasInsert},
        {"select",states::wasSelect},
        {"show",states::wasShow},
        {"update",states::wasUpdate},
        {"use",states::wasUse},
      };
      return gTerms.count(aWord) ? gTerms[aWord]
                                 : states::wasUnknown;
    }

    //extract count (numeric) from line
    int getQueryCount(const std::string &aLine) {
      std::stringstream temp(aLine);
      std::string theWord;
      int theNumber;
      temp >> theWord >> theWord >> theNumber;
      return theNumber;
    }
    
    //extract first word (l/c) from line...
    std::string getFirst(const std::string &aLine) {
      std::stringstream temp(aLine);
      std::string theFirst;
      temp >> theFirst;
      std::transform(theFirst.begin(),
                     theFirst.end(), theFirst.begin(),
          [](unsigned char c){ return std::tolower(c); });
      return theFirst;
    }
    
    //validates output of assignment2
    bool hwIsValid(std::istream &aStream, CountList &aCounts) {
      bool theResult=true;
      states theState=states::wasUnknown;
      
      std::string theLine;
      while(theResult && aStream) {
        std::getline(aStream, theLine);
        if(theLine.size()>2) {
          //std::cout << theLine << "\n";
          std::string theWord=getFirst(theLine);
          switch(theState) {
            case wasUnknown: theState=getTerm(theWord); break;
            case wasUse:
              if(theWord=="database") {
                theState=wasUnknown;
              }
              break;
            case wasAlter:
            case wasCreate:
            case wasDelete:
            case wasInsert:
            case wasUpdate:
            case wasDrop:
              if(theWord=="query") {
                aCounts.push_back(getQueryCount(theLine));
                theState=wasUnknown;
              }
              else return false;
              break;
            case wasDescribe:
            case wasDump:
            case wasSelect:
            case wasShow:
              if(std::isdigit(theWord[0])) {
                aCounts.push_back(stoi(theWord));
                theState=wasUnknown;
              }
              else if(wasUnknown!=getTerm(theWord)) {
                return false; //unexpected kw...
              }
              break;
            default: break;
          }
        }
      }
      return theResult;
    }
    
    using FileList = std::vector<std::string>;
    
    bool hasFiles(FileList &aFilelist) {
      for(auto theFile : aFilelist) {
        std::string thePath=Config::getDBPath(theFile);
        std::ifstream theStream(thePath);
        if(!theStream) return false;
      }
      return true;
    }
    
    std::string getRandomDBName(char aChar) {
      uint32_t theCount=rand() % 99999;
      return std::string("testdb_"+std::to_string(theCount+1000)+aChar);
    }
    
    //----------------------------------------------

    bool doDBTest() {
      
      std::vector<std::string> theFiles;
      
      char theType='A';
      theFiles.push_back(getRandomDBName(theType));
      theFiles.push_back(getRandomDBName(theType));
      theFiles.push_back(getRandomDBName(theType));

      std::stringstream theStream1;
      theStream1 << "CREATE DATABASE "+theFiles[0] << ";\n";
      theStream1 << "create database "+theFiles[1] << ";\n";
      theStream1 << "CrEaTe dAtABaSe "+theFiles[2] << ";\n";
      theStream1 << "use "+theFiles[0] << ";\n";
      theStream1 << "shoW databaseS;\n";
   
      std::string temp(theStream1.str());
      std::stringstream theInput(temp);
      bool theResult=doScriptTest(theStream1,output) && hasFiles(theFiles);
      if(theResult) {
        std::stringstream theStream2;
        theStream2 << "DRop dataBASE "+theFiles[1] << ";\n";
        theStream2 << "shoW databaseS;\n";
        theStream2 << "dump database "+theFiles[0] << ";\n";
        theStream2 << "drop database "+theFiles[0] << ";\n";
        theStream2 << "drop database "+theFiles[2] << ";\n";

        if((theResult=doScriptTest(theStream2,output))) {
          std::string tempStr=output.str();
          std::stringstream theOutput(tempStr);
          CountList theCounts;
          if((theResult=hwIsValid(theOutput,theCounts))) {
            CountList theOpts{1,1,1,4,0,3,3,0,0};
            theOpts[3]=theCounts[3];
            theOpts[5]=theOpts[3]-1;
            theResult=compareCounts(theCounts,theOpts,6);
          }
          
        }
      }
      return theResult;
    }
    
    void addUsersTable(std::ostream &anOutput) {
      anOutput << "create table Users (";
      anOutput << " id int NOT NULL auto_increment primary key,";
      anOutput << " first_name varchar(50) NOT NULL,";
      anOutput << " last_name varchar(50),";
      anOutput << " zipcode int);\n";
    }

    void addAccountsTable(std::ostream &anOutput) {
      anOutput << "create table Accounts (";
      anOutput << " id int NOT NULL auto_increment primary key,";
      anOutput << " account_type varchar(25) NOT NULL,";
      anOutput << " amount int);\n";
    }
    
    void addBooksTable(std::ostream &anOutput) {
      anOutput << "create table Books (";
      anOutput << " id int NOT NULL auto_increment primary key,";
      anOutput << " title varchar(25) NOT NULL,";
      anOutput << " subtitle varchar(25),";
      anOutput << " user_id int);\n";
    }
      
    void insertUsers(std::ostream &anOut,
                     size_t anOffset, size_t aLimit) {
      static const char* kUsers[]={
        " (\"terry\",\"pratchett\",92124)",
        " (\"ian\",\"tregellis\",92123)",
        " (\"jody\",\"taylor\",92120)",
        " (\"stephen\",\"king\",92125)",
        " (\"ted\",\"chiang\",92120)",
        " (\"pu\",\"cheng\",85023)"
      };
      
      anOut<<"INSERT INTO Users (first_name, last_name, zipcode)";
      
      size_t theSize=sizeof(kUsers)/sizeof(char*);
      size_t theLimit=std::min(theSize, anOffset+aLimit);
      const char* thePrefix=" VALUES";
      for(size_t i=anOffset;i<theLimit;i++) {
        anOut << thePrefix << kUsers[i];
        thePrefix=",";
      }
      anOut << ";\n";
    }
          
    void insertFakeUsers(std::ostream &anOut,
                         size_t aGroupSize,
                         size_t aGroupCount) {
      
      for(size_t theCount=0;theCount<aGroupCount;theCount++) {
        anOut<<"INSERT INTO Users (first_name, last_name, zipcode) VALUES ";
        const char* thePrefix="";
        for(size_t theSize=0;theSize<aGroupSize;theSize++) {
          anOut << thePrefix <<
            '(' << '"' << Fake::People::first_name() << "\"," <<
            '"' << Fake::People::last_name() << "\"," <<
            Fake::Places::zipcode() << ')';
          thePrefix=",";
        }
        anOut << ";\n";
      }
    }
      
    void insertBooks(std::ostream &anOut,
                     size_t anOffset, size_t aLimit) {
      static const char* kBooks[]={
        " (\"The Green Mile\",4)",
        " (\"The Stand\",4)",
        " (\"The Misery\",4)",
        " (\"11/22/63\",4)",
        " (\"The Institute\",4)",
        " (\"The Thief of Time\",1)",
        " (\"The Wintersmith\",1)",
        " (\"The Monster Regiment\",1)",
        " (\"Thud\",1)",
        " (\"The Time Police\",3)",
        " (\"The Mechanical\",2)",
        " (\"The Liberation\",2)",
        " (\"The Rising\",2)",
        " (\"Exhalation\",5)",
      };
      
      anOut<<"INSERT INTO Books (title, user_id)";
      
      size_t theSize=sizeof(kBooks)/sizeof(char*);
      size_t theLimit=std::min(theSize, anOffset+aLimit);
      const char* thePrefix=" VALUES";
      for(size_t i=anOffset;i<theLimit;i++) {
        anOut << thePrefix << kBooks[i];
        thePrefix=",";
      }
      anOut << ";\n";
    }
    
    bool doTablesTest() {
      
      std::string theDBName(getRandomDBName('B'));
  
      std::stringstream theStream1;
      theStream1 << "create database " << theDBName << ";\n";
      theStream1 << "use " << theDBName << ";\n";
      
      addUsersTable(theStream1);
      addAccountsTable(theStream1);
 
      theStream1 << "create table Payments (";
      theStream1 << " id int NOT NULL auto_increment primary key,";
      theStream1 << " user_id int,";
      theStream1 << " paydate int,";
      theStream1 << " amount int);\n";
      
      theStream1 << "show tables;\n";
      theStream1 << "describe Accounts;\n";
      theStream1 << "drop table Accounts;\n";
      theStream1 << "show tables;\n";
      theStream1 << "drop database " << theDBName << ";\n";
      theStream1 << "quit;\n";

      std::string temp(theStream1.str());
      std::stringstream theInput(temp);
      bool theResult=doScriptTest(theInput,output);
      if(theResult) {
        std::string tempStr=output.str();
        std::stringstream theOutput(tempStr);
        CountList theCounts;
        if((theResult=hwIsValid(theOutput,theCounts))) {
          static CountList theOpts{1,0,0,0,4,3,0,2,2};
          theResult=compareCounts(theCounts,theOpts,4);
        }
      }
      return theResult;
    }

    bool doInsertTest() {
 
      std::string theDBName(getRandomDBName('C'));
  
      std::stringstream theStream1;
      theStream1 << "create database " << theDBName << ";\n";
      theStream1 << "use " << theDBName << ";\n";
      
      addUsersTable(theStream1);
      insertUsers(theStream1,0,2);

      theStream1 << "show tables;\n";
      theStream1 << "dump database " << theDBName << ";\n";
      theStream1 << "drop database " << theDBName << ";\n";
      theStream1 << "quit;\n";
      
      std::string temp(theStream1.str());
      std::stringstream theInput(temp);
      bool theResult=doScriptTest(theInput,output);
      if(theResult) {
        std::string tempStr=output.str();
        //std::cout << "output \n" << tempStr << "\n";
        std::stringstream theOutput(tempStr);
        CountList theCounts;
        if((theResult=hwIsValid(theOutput,theCounts))) {
          static CountList theOpts{1,0,2,1,7,1};
          theResult=compareCounts(theCounts,theOpts,4);
        }
      }
      return theResult;
    }

    bool compareCounts(CountList &aList1, CountList &aList2, const size_t aPos) {
      bool theResult=true;
      for(size_t i=0;i<aList1.size();i++) {
        if(theResult) {
          theResult=i==aPos ? aList1[i]<aList2[i] : aList1[i]==aList2[i];
        }
      }
      return theResult;
    }
    
    bool doSelectTest() {
       
      std::string theDBName1(getRandomDBName('D'));
      std::string theDBName2(getRandomDBName('D'));
  
      std::stringstream theStream1;
      theStream1 << "create database " << theDBName2 << ";\n";
      theStream1 << "create database " << theDBName1 << ";\n";
      theStream1 << "use " << theDBName1 << ";\n";
      
      addUsersTable(theStream1);
      insertUsers(theStream1,0,5);

      theStream1 << "use " << theDBName2 << ";\n";
      theStream1 << "dump database " << theDBName1 << ";\n";
      theStream1 << "drop database " << theDBName2 << ";\n";
      theStream1 << "use " << theDBName1 << ";\n";
      theStream1 << "select * from Users where zipcode>92120 order by zipcode limit 2;\n";
      theStream1 << "drop database " << theDBName1 << ";\n";
      theStream1 << "quit;\n";
      
      std::string temp(theStream1.str());
      std::stringstream theInput(temp);
      bool theResult=doScriptTest(theInput,output);
      if(theResult) {
        std::string tempStr=output.str();
        //std::cout << "output \n" << tempStr << "\n";
        std::stringstream theOutput(tempStr);
        CountList theCounts;
        if((theResult=hwIsValid(theOutput,theCounts))) {
          static CountList theOpts{1,1,0,5,10,0,2,1};
          theResult=compareCounts(theCounts,theOpts,4);
        }
      }
      return theResult;
    }

    bool doUpdateTest() {

      std::string theDBName1(getRandomDBName('E'));
      std::string theDBName2(getRandomDBName('E'));

      std::stringstream theStream1;
      theStream1 << "create database " << theDBName2 << ";\n";
      theStream1 << "create database " << theDBName1 << ";\n";
      theStream1 << "use " << theDBName1 << ";\n";
      
      addUsersTable(theStream1);
      insertUsers(theStream1,0,5);

      theStream1 << "use " << theDBName2 << ";\n";
      theStream1 << "drop database " << theDBName2 << ";\n";
      theStream1 << "use " << theDBName1 << ";\n";

      theStream1 << "select * from Users;\n";

      std::string theZip(std::to_string(10000+rand()%75000));

      theStream1 << "update Users set zipcode=" << theZip
                 << " where id=5;\n";
      
      theStream1 << "select * from Users where zipcode="
                 << theZip << ";\n";

      theStream1 << "drop database " << theDBName1 << ";\n";
      theStream1 << "quit;\n";

      std::string temp(theStream1.str());
      std::stringstream theInput(temp);
      bool theResult=doScriptTest(theInput,output);
      if(theResult) {
        std::string tempStr=output.str();
        std::stringstream theOutput(tempStr);
        CountList theCounts;
        if((theResult=hwIsValid(theOutput,theCounts))) {
          static CountList theOpts{1,1,0,5,0,5,1,2,1};
          theResult=compareCounts(theCounts,theOpts,7);
        }
      }
      return theResult;
    }

    bool doDeleteTest() {

      std::string theDBName1(getRandomDBName('F'));
      std::string theDBName2(getRandomDBName('F'));

      std::stringstream theStream1;
      theStream1 << "create database " << theDBName2 << ";\n";
      theStream1 << "create database " << theDBName1 << ";\n";
      theStream1 << "use " << theDBName1 << ";\n";
     
      addUsersTable(theStream1);
      insertUsers(theStream1,0,5);
      
      theStream1 << "use " << theDBName2 << ";\n";
      theStream1 << "drop database " << theDBName2 << ";\n";
      theStream1 << "use " << theDBName1 << ";\n";

      theStream1 << "select * from Users;\n";

      theStream1 << "DELETE from Users where zipcode=92120;\n";
      theStream1 << "select * from Users\n";
      theStream1 << "DELETE from Users where zipcode<92124;\n";
      theStream1 << "select * from Users\n";
      theStream1 << "DELETE from Users where zipcode>92124;\n";
      theStream1 << "select * from Users\n";
      theStream1 << "drop database " << theDBName1 << ";\n";
      theStream1 << "quit;\n";

      std::string temp(theStream1.str());
      std::stringstream theInput(temp);
      bool theResult=doScriptTest(theInput,output);
      if(theResult) {
       std::string tempStr=output.str();
       //std::cout << "output \n" << tempStr << "\n";
       std::stringstream theOutput(tempStr);
       CountList theCounts;
       if((theResult=hwIsValid(theOutput,theCounts))) {
         static CountList theOpts{1,1,0,5,0,5,2,3,1,2,1,1,2};
         theResult=compareCounts(theCounts,theOpts,12);
       }
      }
      return theResult;
    }

    //test dropping a table...
    bool doDropTest() {
 
      std::string theDBName1(getRandomDBName('G'));

      std::stringstream theStream1;
      theStream1 << "create database " << theDBName1 << ";\n";
      theStream1 << "use " << theDBName1 << ";\n";
      
      addAccountsTable(theStream1);
      addUsersTable(theStream1);
      insertUsers(theStream1,0,5);

      theStream1 << "drop table Users;\n";
      theStream1 << "show tables\n";
      theStream1 << "dump database " << theDBName1 << ";\n";
      theStream1 << "drop database " << theDBName1 << ";\n";
      theStream1 << "quit;\n";
      
      std::string temp(theStream1.str());
      std::stringstream theInput(temp);
      bool theResult=doScriptTest(theInput,output);
      if(theResult) {
        std::string tempStr=output.str();
        std::stringstream theOutput(tempStr);
        CountList theCounts;
        if((theResult=hwIsValid(theOutput,theCounts))) {
          static CountList theOpts{1,0,0,5,0,1,12,1};
          theResult=compareCounts(theCounts,theOpts,6);
        }
      }
      return theResult;
    }
    
    bool doIndexTest() {
      std::string theDBName1(getRandomDBName('H'));
      std::string theDBName2(getRandomDBName('H'));
      
      //theDBName1="bar";
      std::stringstream theStream1;
      theStream1 << "create database " << theDBName2 << ";\n";
      theStream1 << "create database " << theDBName1 << ";\n";
      theStream1 << "use " << theDBName1 << ";\n";

      addUsersTable(theStream1);
      insertUsers(theStream1,0,5);

      theStream1 << "use " << theDBName2 << ";\n";
      theStream1 << "drop database " << theDBName2 << ";\n";
      theStream1 << "use " << theDBName1 << ";\n";

      insertFakeUsers(theStream1,30,5);

      theStream1 << "show index id from Users;\n";
      theStream1 << "drop database " << theDBName1 << ";\n";
      theStream1 << "quit;\n";

      std::string temp(theStream1.str());
      std::stringstream theInput(temp);
      bool theResult=doScriptTest(theInput,output);
      if(theResult) {
        std::string tempStr=output.str();
        //std::cout << "output \n" << tempStr << "\n";
        std::stringstream theOutput(tempStr);
        CountList theCounts;
        if((theResult=hwIsValid(theOutput,theCounts))) {
          static CountList theOpts{1,1,0,5,0,30,30,30,30,30,155,2};
          theResult=compareCounts(theCounts,theOpts,11);
        }
      }
      return theResult;
    }
    
    bool doJoinTest() {

      std::string theDBName1(getRandomDBName('J'));
      std::stringstream theStream1;
      theStream1 << "create database " << theDBName1 << ";\n";
      theStream1 << "use " << theDBName1 << ";\n";
      
      addUsersTable(theStream1);
      insertUsers(theStream1,0,6);
      addBooksTable(theStream1);
      insertBooks(theStream1,0,14);

      theStream1 << "select * from Users order by last_name;\n";

      theStream1 << "select * from Books order by title;\n";

      theStream1 << "select first_name, last_name, title from Users left join Books on Users.id=Books.user_id order by last_name;\n";
      
      theStream1 << "drop database " << theDBName1 << ";\n";
      theStream1 << "quit;\n";
      
      std::string temp(theStream1.str());
      std::stringstream theInput(temp);
      bool theResult=doScriptTest(theInput,output);
      if(theResult) {
        std::string tempStr=output.str();
        //std::cout << "output \n" << tempStr << "\n";
        std::stringstream theOutput(tempStr);
        CountList theCounts;
        if((theResult=hwIsValid(theOutput,theCounts))) {
          static CountList theOpts{1,0,6,0,14,6,14,15,4};
          theResult=compareCounts(theCounts,theOpts,8);
        }
      }
      return theResult;
      
    }

    bool doCacheTest() {
      bool theResult=false;
      return theResult;
    }

    bool doAlterTest() {
 
      std::stringstream theStream1;
      theStream1 << "create database finalA;\n";
      theStream1 << "create database finalB;\n";
      theStream1 << "use finalA;\n";
      
      addBooksTable(theStream1);
      insertBooks(theStream1,0,14);

      theStream1 << "ALTER TABLE Books add pub_year varchar(4);\n";
      theStream1 << "ALTER TABLE Books drop subtitle;\n";

      theStream1 << "use finalB;\n";
      theStream1 << "drop database finalB;\n";
      theStream1 << "use finalA;\n";

      theStream1 << "UPDATE Books set pub_year=2010 where id<8;\n";
      theStream1 << "describe Books;\n";
      theStream1 << "select * from Books where id<8 order by title;\n";
      theStream1 << "drop database finalA;\n";
      theStream1 << "quit;\n";
      
      std::string temp(theStream1.str());
      std::stringstream theInput(temp);
      bool theResult=doScriptTest(theInput,output);
      if(theResult) {
        std::string tempStr=output.str();
        std::stringstream theOutput(tempStr);
        CountList theCounts;
        if((theResult=hwIsValid(theOutput,theCounts))) {
          static CountList theOpts{1,1,0,14,0,0,0,7,4,7,2};
          theResult=compareCounts(theCounts,theOpts,10);
        }
      }
      return theResult;
    }
    
  };

}

#endif /* TestAutomatic_h */
