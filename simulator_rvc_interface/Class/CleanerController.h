#pragma once

#include "HDWARE/HwCleaner.h"
#include "rvc/Types.hpp"

class EventBus;
class Timer;

class CleanerController {
public:
    CleanerController(EventBus* bus, HwCleaner* cleaner, Timer* timer);

    void applyCommand(const rvc::Command& command);

private:
    HwCleaner* cleaner_;
    Timer* timer_;
};
