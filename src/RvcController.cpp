#include "rvc/RvcController.hpp"

namespace rvc {

RvcController::RvcController(ControllerConfig config) : cleaningPolicy_(config) {}

void RvcController::startCleaning() {
    running_ = true;
    navigationPolicy_.startCleaning();
    sensorFusion_.clearFrontObstacleInterrupt();
}

void RvcController::stopCleaning() {
    running_ = false;
    navigationPolicy_.stopCleaning();
    sensorFusion_.clearFrontObstacleInterrupt();
    cleaningPolicy_.reset();
}

void RvcController::onFrontObstacleInterrupt() {
    if (running_) {
        sensorFusion_.recordFrontObstacleInterrupt();
    }
}

Command RvcController::tick(const PeriodicSensorData& periodicSensors) {
    if (!running_) {
        return commandComposer_.compose(Motion::Stop, CleaningPower::Off, "controller idle");
    }

    const SensorSnapshot snapshot = readPeriodicSensors(periodicSensors);
    Command command = decideNextCommand(snapshot);
    sensorFusion_.clearFrontObstacleInterrupt();
    return command;
}

SensorSnapshot RvcController::readPeriodicSensors(const PeriodicSensorData& periodicSensors) const {
    return sensorFusion_.fuse(periodicSensors);
}

Command RvcController::decideNextCommand(const SensorSnapshot& snapshot) {
    if (!running_) {
        return commandComposer_.compose(Motion::Stop, CleaningPower::Off, "controller idle");
    }

    const CleaningPower power = cleaningPolicy_.update(snapshot.dustDetected);
    const NavigationDecision navigation = navigationPolicy_.decide(snapshot);
    return commandComposer_.compose(navigation.motion, power, navigation.reason);
}

ControllerState RvcController::state() const {
    return navigationPolicy_.state();
}

bool RvcController::isRunning() const {
    return running_;
}

int RvcController::boostTicksRemaining() const {
    return cleaningPolicy_.boostTicksRemaining();
}

}  // namespace rvc
