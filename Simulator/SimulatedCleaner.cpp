#include "Simulator/SimulatedCleaner.h"
#include "Simulator/SimulatedMotor.h"

SimulatedCleaner::SimulatedCleaner(GridMap* map, SimulatedMotor* motor)
    : map(map), motor(motor), turnedOn(false), powerUpMode(false) {
}

void SimulatedCleaner::powerUp() {
    if (turnedOn) {
        powerUpMode = true;
    }
}

void SimulatedCleaner::powerRestore() {
    powerUpMode = false;
}

void SimulatedCleaner::turnOn() {
    turnedOn = true;
}

void SimulatedCleaner::turnOff() {
    turnedOn = false;
    powerUpMode = false;
}

bool SimulatedCleaner::isTurnedOn() const {
    return turnedOn;
}

bool SimulatedCleaner::isPowerUp() const {
    return powerUpMode;
}

bool SimulatedCleaner::cleanCurrentCell() {
    if (!turnedOn || !map || !motor) {
        return false;
    }

    return map->clean(motor->getPosition());
}

void SimulatedCleaner::reset() {
    turnedOn = false;
    powerUpMode = false;
}
