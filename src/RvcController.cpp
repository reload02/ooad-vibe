#include "rvc/RvcController.hpp"

#include <utility>

namespace rvc {

namespace {

[[nodiscard]] Command makeCommand(Motion motion, CleaningPower power, std::string reason) {
    const CleaningPower outputPower = motion == Motion::Forward ? power : CleaningPower::Off;

    return Command{
        .motion = motion,
        .cleaningPower = outputPower,
        .reason = std::move(reason),
    };
}

}  // namespace

RvcController::RvcController(ControllerConfig config) : cleaningPower_(config.dustBoostTicks) {}

void RvcController::startCleaning() {
    running_ = true;
    navigation_.startCleaning();
    frontInterruptPending_ = false;
}

void RvcController::stopCleaning() {
    running_ = false;
    navigation_.stopCleaning();
    frontInterruptPending_ = false;
    cleaningPower_.reset();
}

void RvcController::onFrontObstacleInterrupt() {
    if (running_) {
        frontInterruptPending_ = true;
    }
}

Command RvcController::tick(const PeriodicSensorData& periodicSensors) {
    if (!running_) {
        return makeCommand(Motion::Stop, CleaningPower::Off, "controller idle");
    }

    const SensorSnapshot snapshot = readPeriodicSensors(periodicSensors);
    Command command = decideNextCommand(snapshot);
    frontInterruptPending_ = false;
    return command;
}

SensorSnapshot RvcController::readPeriodicSensors(const PeriodicSensorData& periodicSensors) const {
    return SensorSnapshot{
        .frontObstacle = frontInterruptPending_,
        .leftObstacle = periodicSensors.leftObstacle,
        .rightProbe = navigation_.rightProbeState(),
        .dustDetected = periodicSensors.dustDetected,
    };
}

Command RvcController::decideNextCommand(const SensorSnapshot& snapshot) {
    if (!running_) {
        return makeCommand(Motion::Stop, CleaningPower::Off, "controller idle");
    }

    const CleaningPower power = cleaningPower_.update(snapshot.dustDetected);
    const NavigationDecision navigationDecision = navigation_.decide(snapshot);
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
