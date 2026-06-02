#pragma once

#include "HDWARE/HwCleaner.h"
#include "Simulator/GridMap.h"

class SimulatedMotor;

class SimulatedCleaner : public HwCleaner {
public:
    SimulatedCleaner(GridMap* map, SimulatedMotor* motor);

    void powerUp() override;
    void powerRestore() override;
    void turnOn() override;
    void turnOff() override;

    bool isTurnedOn() const;
    bool isPowerUp() const;
    bool cleanCurrentCell();
    void reset();

private:
    GridMap* map;
    SimulatedMotor* motor;
    bool turnedOn;
    bool powerUpMode;
};
