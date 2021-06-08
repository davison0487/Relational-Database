//
//  Timer.hpp
//  RGAssignment6
//
//  Created by rick gessner on 5/17/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef Timer_h
#define Timer_h

#include <ctime>
#include <chrono>
#include <iostream>
#include <iomanip>

class Timer {
public:
  Timer() {
    stopped=started=std::chrono::high_resolution_clock::now();
  };
  
  Timer(const Timer& aTimer) {
      *this = aTimer;
  }

  Timer& operator=(const Timer& aTimer) {
      stopped = aTimer.stopped;
      started = aTimer.started;
      return *this;
  }

  Timer& start() {
    started=std::chrono::high_resolution_clock::now();
    return *this;
  }
  
  Timer& stop() {
    stopped=std::chrono::high_resolution_clock::now();
    return *this;
  }
  
  double elapsed() {
    if(started!=stopped) {
      std::chrono::duration<double> elapsed = stopped - started;
      return elapsed.count();
    }
    return 0.0;
  }

  void showElapsedTime(std::ostream& anOutput) {
      anOutput << '(' << std::fixed << std::setprecision(4) 
          << elapsed() << " sec)\n\n";
  }
  
  std::chrono::time_point<std::chrono::high_resolution_clock> started;
  std::chrono::time_point<std::chrono::high_resolution_clock> stopped;
};

#endif /* Timer_h */
