#pragma once

#include "rvc/CleaningPowerPolicy.hpp"
#include "rvc/NavigationPolicy.hpp"
#include "rvc/Types.hpp"

namespace rvc {

struct ControllerConfig {
    int dustBoostTicks{3};
};

class RvcController {
public:
    explicit RvcController(ControllerConfig config = {});

    void startCleaning();
    void stopCleaning();
    void onFrontObstacleInterrupt();
    void onBackwardObstacleInterrupt();

    [[nodiscard]] Command tick(const PeriodicSensorData& periodicSensors);
    [[nodiscard]] SensorSnapshot readPeriodicSensors(const PeriodicSensorData& periodicSensors) const;
    [[nodiscard]] Command decideNextCommand(const SensorSnapshot& snapshot);

    [[nodiscard]] ControllerState state() const;
    [[nodiscard]] RightProbeState rightProbeState() const;
    [[nodiscard]] bool isRunning() const;
    [[nodiscard]] int boostTicksRemaining() const;

private:
    NavigationPolicy navigation_;
    CleaningPowerPolicy cleaningPower_;
    bool running_{false};
    bool frontInterruptPending_{false};
    bool backwardInterruptPending_{false};
};

}  // namespace rvc
