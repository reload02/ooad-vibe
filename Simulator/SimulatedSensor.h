#pragma once

#include "Class/ISensor.h"
#include "Simulator/GridMap.h"

class SimulatedMotor;

enum class SensorDirection {
    Front,
    Left,
    Right,
    Dust
};

class SimulatedSensor : public ISensor {
public:
    enum class FaultMode {
        Normal,
        StuckFalse,
        StuckTrue
    };

    SimulatedSensor(GridMap* map, SimulatedMotor* motor, SensorDirection direction);

    bool detect() override;
    bool peek() const;
    void setFaultMode(FaultMode mode);
    FaultMode getFaultMode() const;

private:
    GridMap* map;
    SimulatedMotor* motor;
    SensorDirection direction;
    FaultMode faultMode;

    Point getTargetPoint() const;
};
