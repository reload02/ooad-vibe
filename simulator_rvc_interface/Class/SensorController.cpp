#include "Class/SensorController.h"

#include "Class/EventBus.h"

SensorController::SensorController(EventBus* bus, ISensor* leftSensor, ISensor* dustSensor)
    : bus_(bus), leftSensor_(leftSensor), dustSensor_(dustSensor) {
}

void SensorController::turnOn() {
    enabled_ = true;
}

void SensorController::turnOff() {
    enabled_ = false;
}

void SensorController::ChecknPowerUp() {
    if (!enabled_ || bus_ == nullptr) {
        return;
    }

    const bool leftObstacle = leftSensor_ != nullptr && leftSensor_->detect();
    const bool dustDetected = dustSensor_ != nullptr && dustSensor_->detect();
    bus_->updatePeriodicSensors(leftObstacle, dustDetected);
}

void SensorController::FrontObstacleDetected() {
    if (enabled_ && bus_ != nullptr) {
        bus_->publishFrontObstacleDetected();
    }
}
