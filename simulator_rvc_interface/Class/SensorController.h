#pragma once

#include "Class/ISensor.h"

class EventBus;

class SensorController {
public:
    SensorController(EventBus* bus, ISensor* leftSensor, ISensor* dustSensor);

    void turnOn();
    void turnOff();
    void ChecknPowerUp();
    void FrontObstacleDetected();

private:
    EventBus* bus_;
    ISensor* leftSensor_;
    ISensor* dustSensor_;
    bool enabled_{false};
};
