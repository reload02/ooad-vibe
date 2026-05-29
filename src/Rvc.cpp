#include "rvc/Rvc.hpp"

namespace rvc {

Rvc::Rvc(ControllerConfig config) : controller_(config) {}

void Rvc::startCleaning() {
    controller_.startCleaning();
}

void Rvc::stopCleaning() {
    controller_.stopCleaning();
}

void Rvc::onFrontObstacleInterrupt() {
    controller_.onFrontObstacleInterrupt();
}

Command Rvc::tick(const PeriodicSensorData& periodicSensors) {
    return controller_.tick(periodicSensors);
}

SensorSnapshot Rvc::readPeriodicSensors(const PeriodicSensorData& periodicSensors) const {
    return controller_.readPeriodicSensors(periodicSensors);
}

ControllerState Rvc::state() const {
    return controller_.state();
}

bool Rvc::isRunning() const {
    return controller_.isRunning();
}

int Rvc::boostTicksRemaining() const {
    return controller_.boostTicksRemaining();
}

}  // namespace rvc
