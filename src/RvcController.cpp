#include "rvc/RvcController.hpp"

#include <utility>

namespace rvc {

namespace {

[[nodiscard]] Command makeCommand(Motion motion, CleaningPower power, std::string reason) {
    return Command{
        .motion = motion,
        .cleaningPower = power,
        .reason = std::move(reason),
    };
}

}  // namespace

RvcController::RvcController(ControllerConfig config) : cleaningPower_(config.dustBoostTicks) {}

void RvcController::startCleaning() {
    running_ = true;
    navigation_.startCleaning();
    frontInterruptPending_ = false;
    backwardInterruptPending_ = false;
}

void RvcController::stopCleaning() {
    running_ = false;
    navigation_.stopCleaning();
    frontInterruptPending_ = false;
    backwardInterruptPending_ = false;
    cleaningPower_.reset();
}

void RvcController::onFrontObstacleInterrupt() {
    if (running_) {
        frontInterruptPending_ = true;
    }
}

void RvcController::onBackwardObstacleInterrupt() {
    if (running_) {
        backwardInterruptPending_ = true;
    }
}

Command RvcController::tick(const PeriodicSensorData& periodicSensors) {
    if (!running_) {
        return makeCommand(Motion::Stop, CleaningPower::Off, "controller idle");
    }

    const SensorSnapshot snapshot = readPeriodicSensors(periodicSensors);
    Command command = decideNextCommand(snapshot);
    frontInterruptPending_ = false;
    backwardInterruptPending_ = false;
    return command;
}

SensorSnapshot RvcController::readPeriodicSensors(const PeriodicSensorData& periodicSensors) const {
    return SensorSnapshot{
        .frontObstacle = frontInterruptPending_,
        .backwardObstacle = backwardInterruptPending_,
        .leftObstacle = periodicSensors.leftObstacle,
        .rightProbe = navigation_.rightProbeState(),
        .dustDetected = periodicSensors.dustDetected,
    };
}

Command RvcController::decideNextCommand(const SensorSnapshot& snapshot) {
    if (!running_) {
        return makeCommand(Motion::Stop, CleaningPower::Off, "controller idle");
    }

    const NavigationDecision navigationDecision = navigation_.decide(snapshot);
    const CleaningPower power = cleaningPower_.update(navigation_.state(), snapshot.dustDetected);
    return makeCommand(navigationDecision.motion, power, navigationDecision.reason);
}

ControllerState RvcController::state() const {
    return navigation_.state();
}

RightProbeState RvcController::rightProbeState() const {
    return navigation_.rightProbeState();
}

bool RvcController::isRunning() const {
    return running_;
}

int RvcController::boostTicksRemaining() const {
    return cleaningPower_.boostTicksRemaining();
}

}  // namespace rvc
