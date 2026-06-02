#include "Simulator/SimulatedMotor.h"

SimulatedMotor::SimulatedMotor(GridMap* map)
    : map(map), blocked(false), broken(false), position(1, 1), facing(0, 1) {
}

Point SimulatedMotor::getPosition() const {
    return position;
}

Point SimulatedMotor::getFacing() const {
    return facing;
}

void SimulatedMotor::moveForward() {
    if (broken) {
        blocked = true;
        return;
    }
    tryMove(Point(position.x + facing.x, position.y + facing.y));
}

void SimulatedMotor::moveBackward() {
    if (broken) {
        blocked = true;
        return;
    }
    tryMove(Point(position.x - facing.x, position.y - facing.y));
}

void SimulatedMotor::turnLeft() {
    if (broken) {
        blocked = true;
        return;
    }
    Point temp = facing;
    facing.x = -temp.y;
    facing.y = temp.x;
}

void SimulatedMotor::turnRight() {
    if (broken) {
        blocked = true;
        return;
    }
    Point temp = facing;
    facing.x = temp.y;
    facing.y = -temp.x;
}

bool SimulatedMotor::wasBlocked() const {
    return blocked;
}

void SimulatedMotor::clearBlocked() {
    blocked = false;
}

void SimulatedMotor::resetPose(Point newPosition, Point newFacing) {
    position = newPosition;
    facing = newFacing;
    blocked = false;
    broken = false;
}

void SimulatedMotor::setBroken(bool brokenState) {
    broken = brokenState;
}

bool SimulatedMotor::isBroken() const {
    return broken;
}

bool SimulatedMotor::tryMove(Point next) {
    if (map && map->isWall(next)) {
        blocked = true;
        return false;
    }

    position = next;
    blocked = false;
    return true;
}
