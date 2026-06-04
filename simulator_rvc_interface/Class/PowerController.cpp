#include "Class/PowerController.h"

#include "Class/EventBus.h"

PowerController::PowerController(EventBus* bus) : bus_(bus) {
}

void PowerController::turnOn() {
    powerOn_ = true;
    if (bus_ != nullptr) {
        bus_->setPowerOn(true);
    }
}

void PowerController::turnOff() {
    powerOn_ = false;
    if (bus_ != nullptr) {
        bus_->setPowerOn(false);
        bus_->publishStopCleaning();
    }
}

bool PowerController::isPowerOn() const {
    return powerOn_;
}
