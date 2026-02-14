#include "measurement_manager.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "aliasing.h"
#include "measurement_aggregator.h"

namespace rc {

void MeasurementManager::StartMeasuring(i64 frame_count) {
  for (auto* aggregator : aggregators_) {
    aggregator->StartMeasuring(frame_count);
  }
}

void MeasurementManager::StopMeasuring() {
  for (auto* aggregator : aggregators_) {
    aggregator->StopMeasuring();
  }
}

void MeasurementManager::AddObserver(MeasurementAggregator* aggregator) {
  aggregators_.push_back(aggregator);
}

void MeasurementManager::RemoveObserver(MeasurementAggregator* aggregator) {
  if (auto search =
        std::find(aggregators_.begin(), aggregators_.end(), aggregator);
      search != aggregators_.end()) {
    aggregators_.erase(search);
  }
}

void MeasurementManager::SaveResults() const {
  std::stringstream ss;
  for (MeasurementAggregator* aggregator : aggregators_) {
    aggregator->SaveResults(ss);
  }

  std::string path = std::format(
    "Measurement_{}",
    std::chrono::current_zone()->to_local(std::chrono::system_clock::now()));
  path.erase(remove_if(path.begin(), path.end(),
                       [](char c) {
                         return c == ' ' || c == '.' || c == ':';
                       }),
             path.end());
  path += ".stats";
  std::ofstream output_file(path);
  output_file << ss.rdbuf();
  output_file.close();
}

}; // namespace rc
