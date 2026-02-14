#ifndef MEASUREMENT_MANAGER_H_
#define MEASUREMENT_MANAGER_H_

#include <vector>

#include "aliasing.h"

namespace rc {
class MeasurementAggregator;

class MeasurementManager {
  public:
    void StartMeasuring(i64 frame_count);
    void StopMeasuring();
    void SaveResults() const;

    void AddObserver(MeasurementAggregator* aggregator);
    void RemoveObserver(MeasurementAggregator* aggregator);

  private:
    std::vector<MeasurementAggregator*> aggregators_;
};

} // namespace rc

#endif //! MEASUREMENT_MANAGER_H_
