#include "rvc/RvcController.hpp"

#include <utility>

namespace rvc {

RvcController::RvcController(ControllerConfig config) : config_(config) {}

void RvcController::startCleaning() {
    running_ = true;
    state_ = ControllerState::Cleaning;
    frontInterruptPending_ = false;
}

void RvcController::stopCleaning() {
    running_ = false;
    state_ = ControllerState::Idle;
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
        .rightObstacle = periodicSensors.rightObstacle,
        .dustDetected = periodicSensors.dustDetected,
    };
}

Command RvcController::decideNextCommand(const SensorSnapshot& snapshot) {
    if (!running_) {
        return makeCommand(Motion::Stop, CleaningPower::Off, "controller idle");
    }

    const CleaningPower power = updateCleaningPower(snapshot.dustDetected);
    const bool sidesBlocked = snapshot.leftObstacle && snapshot.rightObstacle;
    const bool allBlocked = snapshot.frontObstacle && sidesBlocked;

    if (state_ == ControllerState::Escaping) {
        if (sidesBlocked) {
            return makeCommand(Motion::Backward, power, "escaping: keep backing up until a side exit opens");
        }

        state_ = ControllerState::Avoiding;
        return makeCommand(chooseOpenSideTurn(snapshot.leftObstacle, snapshot.rightObstacle), power,
                           "escaping: side opened, turn toward exit");
    }

    if (!snapshot.frontObstacle) {
        state_ = ControllerState::Cleaning;
        return makeCommand(Motion::Forward, power, "front clear: forward cleaning");
    }

    if (allBlocked) {
        state_ = ControllerState::Escaping;
        return makeCommand(Motion::Backward, power, "front interrupt and both sides blocked: enter escaping");
    }

    state_ = ControllerState::Avoiding;
    return makeCommand(chooseOpenSideTurn(snapshot.leftObstacle, snapshot.rightObstacle), power,
                       "front interrupt: stop forward motion and turn");
}

ControllerState RvcController::state() const {
    return state_;
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

Motion RvcController::chooseOpenSideTurn(bool leftObstacle, bool rightObstacle) {
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

Command RvcController::makeCommand(Motion motion, CleaningPower power, std::string reason) const {
    const CleaningPower outputPower = motion == Motion::Forward ? power : CleaningPower::Off;

    return Command{
        .motion = motion,
        .cleaningPower = outputPower,
        .reason = std::move(reason),
    };
}

}  // namespace rvc
