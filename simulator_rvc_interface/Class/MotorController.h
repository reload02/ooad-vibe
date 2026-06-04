#pragma once

#include "HDWARE/Motor.h"

class EventBus;

class MotorController {
public:
    MotorController(EventBus* bus, Motor& motor);

    void turnOn();
    void turnOff();
    void MCMove();

    [[nodiscard]] bool isAvoiding() const;

private:
    EventBus* bus_;
    Motor* motor_;
    bool enabled_{false};
};
