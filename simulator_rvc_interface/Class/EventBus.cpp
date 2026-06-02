#include "Class/EventBus.h"

#include "Class/Timer.h"

void EventBus::attachCleaner(HwCleaner* cleaner, Timer* timer) {
    cleaner_ = cleaner;
    timer_ = timer;
}

void EventBus::setPowerOn(bool powerOn) {
    powerOn_ = powerOn;
}

bool EventBus::isPowerOn() const {
    return powerOn_;
}

void EventBus::publishStartCleaning() {
    controller_.startCleaning();
}

void EventBus::publishStopCleaning() {
    controller_.stopCleaning();
    lastCommand_ = rvc::Command{
        .motion = rvc::Motion::Stop,
        .cleaningPower = rvc::CleaningPower::Off,
        .reason = "power off",
    };
    applyCleanerCommand(lastCommand_);
}

void EventBus::publishFrontObstacleDetected() {
    controller_.onFrontObstacleInterrupt();
}

void EventBus::updatePeriodicSensors(bool leftObstacle, bool dustDetected) {
    periodicSensors_ = rvc::PeriodicSensorData{
        .leftObstacle = leftObstacle,
        .dustDetected = dustDetected,
    };
}

rvc::Command EventBus::runControllerTick() {
    lastCommand_ = controller_.tick(periodicSensors_);
    applyCleanerCommand(lastCommand_);
    return lastCommand_;
}

const rvc::Command& EventBus::lastCommand() const {
    return lastCommand_;
}

rvc::ControllerState EventBus::controllerState() const {
    return controller_.state();
}

bool EventBus::isControllerRunning() const {
    return controller_.isRunning();
}

bool EventBus::isAvoiding() const {
    switch (controller_.state()) {
        case rvc::ControllerState::Avoiding:
        case rvc::ControllerState::EscapeAligning:
        case rvc::ControllerState::Escaping:
            return true;
        case rvc::ControllerState::Idle:
        case rvc::ControllerState::Cleaning:
        case rvc::ControllerState::RightProbing:
            return false;
    }
    return false;
}

void EventBus::applyCleanerCommand(const rvc::Command& command) {
    if (cleaner_ == nullptr) {
        return;
    }

    switch (command.cleaningPower) {
        case rvc::CleaningPower::Off:
            cleaner_->powerRestore();
            cleaner_->turnOff();
            break;
        case rvc::CleaningPower::Normal:
            cleaner_->turnOn();
            cleaner_->powerRestore();
            break;
        case rvc::CleaningPower::Boost:
            cleaner_->turnOn();
            cleaner_->powerUp();
            break;
    }

    (void)timer_;
}
