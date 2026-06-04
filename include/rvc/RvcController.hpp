#pragma once

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

    [[nodiscard]] Command tick(const PeriodicSensorData& periodicSensors);
    [[nodiscard]] SensorSnapshot readPeriodicSensors(const PeriodicSensorData& periodicSensors) const;
    [[nodiscard]] Command decideNextCommand(const SensorSnapshot& snapshot);

    [[nodiscard]] ControllerState state() const;
    [[nodiscard]] RightProbeState rightProbeState() const;
    [[nodiscard]] bool isRunning() const;
    [[nodiscard]] int boostTicksRemaining() const;

private:
    [[nodiscard]] CleaningPower updateCleaningPower(bool dustDetected);
    [[nodiscard]] Command makeCommand(Motion motion, CleaningPower power, std::string reason) const;

    ControllerConfig config_;
    ControllerState state_{ControllerState::Idle};
    RightProbeState rightProbe_{RightProbeState::None};
    bool running_{false};
    bool frontInterruptPending_{false};
    int boostTicksRemaining_{0};
};

}  // namespace rvc
