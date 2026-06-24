#ifndef RADIANCE_CASCADES_BOUNCERS_SIMULATION_H_
#define RADIANCE_CASCADES_BOUNCERS_SIMULATION_H_

#include <stop_token>
#include <thread>

#include "aliasing.h"

namespace rc {

class Simulation {
 public:
  class State {
    // bounds
    // field effects
    // entities
    // on-entity effects
    // other effects
  };

  Simulation()
      : main_thread_{&Simulation::RunSimulation, this,
                     stop_source_.get_token()} {

        };

 private:
  void RunSimulation(std::stop_token stop_token) {
    while (!stop_token.stop_possible()) {
    }
  }

  std::stop_source stop_source_{};
  std::jthread main_thread_;

  u64 renderer_ticks_per_second_{60};
  u64 simulation_ticks_per_second_{120};
};

}  // namespace rc

#endif  // ! RADIANCE_CASCADES_BOUNCERS_SIMULATION_H_
