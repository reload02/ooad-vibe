#include "rvc/RvcSubsystems.hpp"

#include <utility>

namespace rvc {

void SensorFusion::recordFrontObstacleInterrupt() {
    frontInterruptPending_ = true;
}

void SensorFusion::clearFrontObstacleInterrupt() {
    frontInterruptPending_ = false;
}

SensorSnapshot SensorFusion::fuse(const PeriodicSensorData& periodicSensors) const {
    return SensorSnapshot{
        .frontObstacle = frontInterruptPending_,
        .leftObstacle = periodicSensors.leftObstacle,
        .rightObstacle = periodicSensors.rightObstacle,
        .dustDetected = periodicSensors.dustDetected,
    };
}

bool SensorFusion::frontInterruptPending() const {
    return frontInterruptPending_;
}

void NavigationPolicy::startCleaning() {
    state_ = ControllerState::Cleaning;
}

void NavigationPolicy::stopCleaning() {
    state_ = ControllerState::Idle;
}

NavigationDecision NavigationPolicy::decide(const SensorSnapshot& snapshot) {
    const bool sidesBlocked = snapshot.leftObstacle && snapshot.rightObstacle;
    const bool allBlocked = snapshot.frontObstacle && sidesBlocked;

    if (state_ == ControllerState::Escaping) {
        if (sidesBlocked) {
            return NavigationDecision{
                .motion = Motion::Backward,
                .reason = "escaping: keep backing up until a side exit opens",
            };
        }

        state_ = ControllerState::Avoiding;
        return NavigationDecision{
            .motion = chooseOpenSideTurn(snapshot.leftObstacle, snapshot.rightObstacle),
            .reason = "escaping: side opened, turn toward exit",
        };
    }

    if (!snapshot.frontObstacle) {
        state_ = ControllerState::Cleaning;
        return NavigationDecision{
            .motion = Motion::Forward,
            .reason = "front clear: forward cleaning",
        };
    }

    if (allBlocked) {
        state_ = ControllerState::Escaping;
        return NavigationDecision{
            .motion = Motion::Backward,
            .reason = "front interrupt and both sides blocked: enter escaping",
        };
    }

    state_ = ControllerState::Avoiding;
    return NavigationDecision{
        .motion = chooseOpenSideTurn(snapshot.leftObstacle, snapshot.rightObstacle),
        .reason = "front interrupt: stop forward motion and turn",
    };
}

ControllerState NavigationPolicy::state() const {
    return state_;
}

Motion NavigationPolicy::chooseOpenSideTurn(bool leftObstacle, bool rightObstacle) {
    if (!leftObstacle && rightObstacle) {
        return Motion::TurnLeft;
    }

    if (leftObstacle && !rightObstacle) {
        return Motion::TurnRight;
    }

    if (!leftObstacle && !rightObstacle) {
        const Motion selected = preferLeftTurn_ ? Motion::TurnLeft : Motion::TurnRight;
        preferLeftTurn_ = !preferLeftTurn_;
        return selected;
    }

    return Motion::Backward;
}

CleaningPolicy::CleaningPolicy(ControllerConfig config) : config_(config) {}

void CleaningPolicy::reset() {
    boostTicksRemaining_ = 0;
}

CleaningPower CleaningPolicy::update(bool dustDetected) {
    if (dustDetected) {
        boostTicksRemaining_ = config_.dustBoostTicks;
    } else if (boostTicksRemaining_ > 0) {
        --boostTicksRemaining_;
    }

    return boostTicksRemaining_ > 0 ? CleaningPower::Boost : CleaningPower::Normal;
}

int CleaningPolicy::boostTicksRemaining() const {
    return boostTicksRemaining_;
}

Command CommandComposer::compose(Motion motion, CleaningPower power, std::string reason) const {
    const CleaningPower outputPower = motion == Motion::Forward ? power : CleaningPower::Off;

    return Command{
        .motion = motion,
        .cleaningPower = outputPower,
        .reason = std::move(reason),
    };
}

}  // namespace rvc
