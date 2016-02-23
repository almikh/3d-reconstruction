#ifndef TIMER_H_INCLUDED__
#define TIMER_H_INCLUDED__

#define __STDC_LIMIT_MACROS 
#include <chrono>

namespace ip {
  using namespace std::chrono;

  class Timer {
    high_resolution_clock::time_point last_call_;

  public:
    Timer();

    void tic();
    long long toc() const;
  };
}

#endif // TIMER_H_INCLUDED__
