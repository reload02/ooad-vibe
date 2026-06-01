#include "rvc/RvcController.hpp"

#include <utility>

namespace rvc {

RvcController::RvcController(ControllerConfig config) : config_(config) {}

void RvcController::startCleaning() {
    running_ = true;
    state_ = ControllerState::Cleaning;
    rightProbe_ = RightProbeState::None;
    frontInterruptPending_ = false;
}

void RvcController::stopCleaning() {
    running_ = false;
    state_ = ControllerState::Idle;
    rightProbe_ = RightProbeState::None;
    frontInterruptPending_ = false;
    boostTicksRemaining_ = 0;
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
        .rightProbe = rightProbe_,
        .dustDetected = periodicSensors.dustDetected,
    };
}

Command RvcController::decideNextCommand(const SensorSnapshot& snapshot) {
    if (!running_) {
        return makeCommand(Motion::Stop, CleaningPower::Off, "controller idle");
    }

    const CleaningPower power = updateCleaningPower(snapshot.dustDetected);

    if (state_ == ControllerState::RightProbing) {
        if (snapshot.frontObstacle) {
            state_ = ControllerState::EscapeAligning;
            rightProbe_ = RightProbeState::Blocked;
            return makeCommand(Motion::TurnLeft, power, "right probe blocked: restore original heading");
        }

        state_ = ControllerState::Cleaning;
        rightProbe_ = RightProbeState::Open;
        return makeCommand(Motion::Forward, power, "right probe open: resume forward cleaning");
    }

    if (state_ == ControllerState::EscapeAligning) {
        state_ = ControllerState::Escaping;
        rightProbe_ = RightProbeState::Blocked;
        return makeCommand(Motion::Backward, power, "right probe blocked: original heading restored, backing up");
    }

    if (state_ == ControllerState::Escaping) {
        if (!snapshot.leftObstacle) {
            state_ = ControllerState::Avoiding;
            rightProbe_ = RightProbeState::None;
            return makeCommand(Motion::TurnLeft, power, "escaping: left opened, turn toward exit");
        }

        state_ = ControllerState::RightProbing;
        rightProbe_ = RightProbeState::Checking;
        return makeCommand(Motion::TurnRight, power, "escaping: left blocked, probe right");
    }

    if (!snapshot.frontObstacle) {
        state_ = ControllerState::Cleaning;
        rightProbe_ = RightProbeState::None;
        return makeCommand(Motion::Forward, power, "front clear: forward cleaning");
    }

    if (!snapshot.leftObstacle) {
        state_ = ControllerState::Avoiding;
        rightProbe_ = RightProbeState::None;
        return makeCommand(Motion::TurnLeft, power, "front interrupt: left open, turn left");
    }

    state_ = ControllerState::RightProbing;
    rightProbe_ = RightProbeState::Checking;
    return makeCommand(Motion::TurnRight, power, "front interrupt: left blocked, probe right");
}

ControllerState RvcController::state() const {
    return state_;
}

RightProbeState RvcController::rightProbeState() const {
    return rightProbe_;
}

bool RvcController::isRunning() const {
    return running_;
}

int RvcController::boostTicksRemaining() const {
    return boostTicksRemaining_;
}

CleaningPower RvcController::updateCleaningPower(bool dustDetected) {
    if (dustDetected) {
        boostTicksRemaining_ = config_.dustBoostTicks;
    } else if (boostTicksRemaining_ > 0) {
        --boostTicksRemaining_;
    }

    return boostTicksRemaining_ > 0 ? CleaningPower::Boost : CleaningPower::Normal;
}

Command RvcController::makeCommand(Motion motion, CleaningPower power, std::string reason) const {
    const CleaningPower outputPower = motion == Motion::Forward ? power : CleaningPower::Off;

    return Command{
        .motion = motion,
        .cleaningPower = outputPower,
        .reason = std::move(reason),
    };
}

}  // namespace rvc
