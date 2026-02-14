#ifndef RC_MEASURE_AGGREGATOR_H_
#define RC_MEASURE_AGGREGATOR_H_

#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "aliasing.h"
#include "app.h"
#include "measurement_manager.h"
#include "scoped_observation.h"

namespace rc {

class MeasurementAggregator {
  public:
    explicit MeasurementAggregator(std::string_view name)
      : name_(name), observation_(this) {
      observation_.Observe(App::Instance().measurement_manager());
    }

    void AddMeasure(i64 record) {
      measurements_.push_back(record);
    };

    void StartMeasuring(u64 frame_count) {
      measurements_.clear();
      measurements_.reserve(frame_count);
      measuring_turned_on_ = true;
    };

    void StopMeasuring() {
      measuring_turned_on_ = false;
    }

    bool ShouldMeasure() const {
      return measuring_turned_on_;
    }

    void SaveResults(std::stringstream& ss) const {
      ss << name_ << " ";
      for (i64 measurement : measurements_) {
        ss << measurement << " ";
      }
      ss << "\n";
    }

  private:
    std::vector<i64> measurements_;
    bool measuring_turned_on_{false};
    std::string name_;
    ScopedObservation<MeasurementAggregator, MeasurementManager> observation_;
};

} // namespace rc

#endif // !RC_MEASURE_AGGREGATOR_H_
