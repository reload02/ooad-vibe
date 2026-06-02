#include "Class/Timer.h"

void Timer::syncTimerDigitalClock() {
    ++ticks_;
}

void Timer::reset() {
    ticks_ = 0;
}

int Timer::ticks() const {
    return ticks_;
}
