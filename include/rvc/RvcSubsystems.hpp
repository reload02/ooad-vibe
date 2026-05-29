#pragma once

#include "rvc/Types.hpp"

#include <string>

namespace rvc {

struct ControllerConfig {
    int dustBoostTicks{3};
};

struct NavigationDecision {
    Motion motion{Motion::None};
    std::string reason;
};

class SensorFusion {
public:
    void recordFrontObstacleInterrupt();
    void clearFrontObstacleInterrupt();

    [[nodiscard]] SensorSnapshot fuse(const PeriodicSensorData& periodicSensors) const;
    [[nodiscard]] bool frontInterruptPending() const;

private:
    bool frontInterruptPending_{false};
};

class NavigationPolicy {
public:
    void startCleaning();
    void stopCleaning();

    [[nodiscard]] NavigationDecision decide(const SensorSnapshot& snapshot);
    [[nodiscard]] ControllerState state() const;

private:
    [[nodiscard]] Motion chooseOpenSideTurn(bool leftObstacle, bool rightObstacle);

    ControllerState state_{ControllerState::Idle};
    bool preferLeftTurn_{true};
};

class CleaningPolicy {
public:
    explicit CleaningPolicy(ControllerConfig config = {});

    void reset();

    [[nodiscard]] CleaningPower update(bool dustDetected);
    [[nodiscard]] int boostTicksRemaining() const;

private:
    ControllerConfig config_;
    int boostTicksRemaining_{0};
};

class CommandComposer {
public:
    [[nodiscard]] Command compose(Motion motion, CleaningPower power, std::string reason) const;
};

}  // namespace rvc
