#include "rvc/Rvc.hpp"

#include <stdexcept>
#include <utility>

namespace rvc {

Rvc::Rvc(std::unique_ptr<RvcHardwareAdapter> hardwareAdapter, ControllerConfig config)
    : controller_(config), hardwareAdapter_(std::move(hardwareAdapter)) {
    if (hardwareAdapter_ == nullptr) {
        throw std::invalid_argument("hardwareAdapter must not be null");
    }
}

void Rvc::startCleaning() {
    controller_.startCleaning();
}

void Rvc::stopCleaning() {
    controller_.stopCleaning();
}

Command Rvc::tick() {
    lastFrontObstacleInterrupt_ = hardwareAdapter_->hasFrontObstacleInterrupt();
    if (lastFrontObstacleInterrupt_) {
        controller_.onFrontObstacleInterrupt();
    }

    lastBackwardObstacleInterrupt_ = hardwareAdapter_->hasBackwardObstacleInterrupt();
    if (lastBackwardObstacleInterrupt_) {
        controller_.onBackwardObstacleInterrupt();
    }

    lastPeriodicSensors_ = hardwareAdapter_->readPeriodicSensors();
    lastCommand_ = controller_.tick(lastPeriodicSensors_);
    hardwareAdapter_->applyCommand(lastCommand_);
    return lastCommand_;
}

bool Rvc::lastFrontObstacleInterrupt() const {
    return lastFrontObstacleInterrupt_;
}

bool Rvc::lastBackwardObstacleInterrupt() const {
    return lastBackwardObstacleInterrupt_;
}

const PeriodicSensorData& Rvc::lastPeriodicSensors() const {
    return lastPeriodicSensors_;
}

const Command& Rvc::lastCommand() const {
    return lastCommand_;
}

}  // namespace rvc
