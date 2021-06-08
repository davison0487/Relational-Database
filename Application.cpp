//
//  CommandProcessor.cpp
//  ECEDatabase
//
//  Created by rick gessner on 3/30/18.
//  Copyright © 2018 rick gessner. All rights reserved.
//

#include <iostream>
#include "Application.hpp"
#include "Tokenizer.hpp"
#include <memory>
#include <vector>
#include <algorithm>


namespace ECE141 {
  
    Application::Application(std::ostream& anOutput)
        : CmdProcessor(anOutput), theDBProc(anOutput) {
    }
  
    Application::~Application() {
        //if(database) delete database;
    }

  // USE: -----------------------------------------------------
  
  static bool isKnown(Keywords aKeyword) {
    static Keywords theKnown[]={Keywords::quit_kw, Keywords::version_kw, Keywords::help_kw };
    auto theIt = std::find(std::begin(theKnown),
                           std::end(theKnown), aKeyword);
    return theIt!=std::end(theKnown);
  }

  CmdProcessor* Application::recognizes(Tokenizer &aTokenizer) {
    if(isKnown(aTokenizer.current().keyword)) {
      return this;
    }
    return theDBProc.recognizes(aTokenizer);
  }
  
  static StatusResult shutdown(std::ostream &anOutput) {
    anOutput << "DB::141 is shutting down.\n";
    return StatusResult(ECE141::userTerminated);
  }

  static StatusResult showHelp(std::ostream& anOutput) {
      anOutput << "Help system ready.\n";
      return StatusResult();
  }

  static StatusResult showVersion(std::ostream& anOutput) {
      anOutput << "Version 0.9.\n";
      return StatusResult();
  }

  StatusResult Application::run(Statement* aStatement, const Timer& aTimer) {
    switch(aStatement->getType()) {
      case Keywords::quit_kw: return shutdown(output);
      case Keywords::help_kw: return showHelp(output);
      case Keywords::version_kw: return showVersion(output);
      default: break;
    }
    return StatusResult{Errors::noError};
  }
  
  // USE: retrieve a statement based on given text input...
  Statement* Application::makeStatement(Tokenizer &aTokenizer, StatusResult& aResult) {
    Token theToken=aTokenizer.current();
    if (isKnown(theToken.keyword)) {
      aTokenizer.next(); //skip ahead...
      return new Statement(theToken.keyword);
    }

    return nullptr;
  }
  
  //build a tokenizer, tokenize input, ask processors to handle...
  StatusResult Application::handleInput(std::istream& anInput) {
      Tokenizer theTokenizer(anInput);
      ECE141::StatusResult theResult = theTokenizer.tokenize();
      while (theResult && theTokenizer.more()) {
          Timer theTimer;
          theTimer.start();
          if (auto* theProc = recognizes(theTokenizer)) {
              if (auto* theCmd = theProc->makeStatement(theTokenizer, theResult)) {
                  theResult = theProc->run(theCmd, theTimer);
                  if (theResult) theTokenizer.skipIf(';');
                  delete theCmd;
              }
          }
          else theTokenizer.next(); //force skip ahead...
      }
      return theResult;
  }
}
