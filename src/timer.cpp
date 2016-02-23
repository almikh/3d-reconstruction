#include <timer.h>

namespace ip {
  Timer::Timer() {
    last_call_ = high_resolution_clock::now();
  }

  void Timer::tic() {
    last_call_ = high_resolution_clock::now();
  }

  long long Timer::toc() const {
    auto diff = high_resolution_clock::now() - last_call_;
    auto ms = duration_cast<milliseconds>(diff);
    return ms.count();
  }
}
