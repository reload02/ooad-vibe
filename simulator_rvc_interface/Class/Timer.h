#pragma once

class Timer {
public:
    void syncTimerDigitalClock();
    void reset();

    [[nodiscard]] int ticks() const;

private:
    int ticks_{0};
};
