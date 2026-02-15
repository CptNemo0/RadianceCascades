#ifndef RC_TIMED_SCOPE_H_
#define RC_TIMED_SCOPE_H_

#include <chrono>

#include "aliasing.h"
#include "measurement_aggregator.h"

namespace rc {

class TimedScope {
  public:
    using clock = std::chrono::high_resolution_clock;

    explicit TimedScope(MeasurementAggregator* aggregator)
      : aggregator_(aggregator), start_(clock::now()) {
    }

    TimedScope(const TimedScope&) = delete;
    TimedScope(TimedScope&&) = delete;

    void operator=(const TimedScope&) = delete;
    void operator=(TimedScope&&) = delete;

    ~TimedScope() {
      if (aggregator_) {
        aggregator_->AddMeasure(Elapsed());
      }
    }

    i64 Elapsed() const {
      return std::chrono::duration_cast<std::chrono::microseconds>(
               clock::now() - start_)
        .count();
    }

  private:
    MeasurementAggregator* aggregator_;
    std::chrono::high_resolution_clock::time_point start_;
};

} // namespace rc

#endif // !RC_TIMED_SCOPE_H_
