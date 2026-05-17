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
    [[nodiscard]] bool isRunning() const;
    [[nodiscard]] int boostTicksRemaining() const;

private:
    [[nodiscard]] CleaningPower updateCleaningPower(bool dustDetected);
    [[nodiscard]] Motion chooseOpenSideTurn(bool leftObstacle, bool rightObstacle);
    [[nodiscard]] Command makeCommand(Motion motion, CleaningPower power, std::string reason) const;

    ControllerConfig config_;
    ControllerState state_{ControllerState::Idle};
    bool running_{false};
    bool frontInterruptPending_{false};
    bool preferLeftTurn_{true};
    int boostTicksRemaining_{0};
};

}  // namespace rvc
