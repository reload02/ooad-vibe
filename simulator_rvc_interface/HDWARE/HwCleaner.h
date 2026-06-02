#pragma once

class HwCleaner {
public:
    virtual ~HwCleaner() = default;

    virtual void powerUp() = 0;
    virtual void powerRestore() = 0;
    virtual void turnOn() = 0;
    virtual void turnOff() = 0;
};
