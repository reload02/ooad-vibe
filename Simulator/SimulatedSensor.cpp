#include "Simulator/SimulatedSensor.h"
#include "Simulator/SimulatedMotor.h"

SimulatedSensor::SimulatedSensor(GridMap* map, SimulatedMotor* motor, SensorDirection direction)
    : map(map), motor(motor), direction(direction), faultMode(FaultMode::Normal) {
}

bool SimulatedSensor::detect() {
    return peek();
}

bool SimulatedSensor::peek() const {
    if (faultMode == FaultMode::StuckFalse) {
        return false;
    }
    if (faultMode == FaultMode::StuckTrue) {
        return true;
    }

    if (!map || !motor) {
        return false;
    }

    if (direction == SensorDirection::Dust) {
        return map->hasDust(motor->getPosition());
    }

    return map->isWall(getTargetPoint());
}

void SimulatedSensor::setFaultMode(FaultMode mode) {
    faultMode = mode;
}

SimulatedSensor::FaultMode SimulatedSensor::getFaultMode() const {
    return faultMode;
}

Point SimulatedSensor::getTargetPoint() const {
    Point facing = motor->getFacing();

    if (direction == SensorDirection::Left) {
        facing = Point(-facing.y, facing.x);
    } else if (direction == SensorDirection::Right) {
        facing = Point(facing.y, -facing.x);
    }

    Point pos = motor->getPosition();
    return Point(pos.x + facing.x, pos.y + facing.y);
}
