#include "Class/CleanerController.h"

#include "Class/EventBus.h"
#include "Class/Timer.h"

CleanerController::CleanerController(EventBus* bus, HwCleaner* cleaner, Timer* timer)
    : cleaner_(cleaner), timer_(timer) {
    if (bus != nullptr) {
        bus->attachCleaner(cleaner, timer);
    }
}

void CleanerController::applyCommand(const rvc::Command& command) {
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
