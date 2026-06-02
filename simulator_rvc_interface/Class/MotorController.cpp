#include "Class/MotorController.h"

#include "Class/EventBus.h"

MotorController::MotorController(EventBus* bus, Motor& motor) : bus_(bus), motor_(&motor) {
}

void MotorController::turnOn() {
    enabled_ = true;
}

void MotorController::turnOff() {
    enabled_ = false;
}

void MotorController::MCMove() {
    if (!enabled_ || bus_ == nullptr || motor_ == nullptr) {
        return;
    }

    const rvc::Command command = bus_->runControllerTick();
    switch (command.motion) {
        case rvc::Motion::Forward:
            motor_->moveForward();
            break;
        case rvc::Motion::Backward:
            motor_->moveBackward();
            break;
        case rvc::Motion::TurnLeft:
            motor_->turnLeft();
            break;
        case rvc::Motion::TurnRight:
            motor_->turnRight();
            break;
        case rvc::Motion::None:
        case rvc::Motion::Stop:
            break;
    }
}

bool MotorController::isAvoiding() const {
    return bus_ != nullptr && bus_->isAvoiding();
}
