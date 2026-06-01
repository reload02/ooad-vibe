#include "rvc/SimulatedHardwareAdapter.hpp"

#include <algorithm>
#include <sstream>
#include <stdexcept>

namespace rvc {

namespace {

char directionMarker(Direction direction) {
    switch (direction) {
        case Direction::North:
            return '^';
        case Direction::East:
            return '>';
        case Direction::South:
            return 'v';
        case Direction::West:
            return '<';
    }
    return '?';
}

Direction directionFromMarker(char marker) {
    switch (marker) {
        case '^':
            return Direction::North;
        case '>':
            return Direction::East;
        case 'v':
            return Direction::South;
        case '<':
            return Direction::West;
        default:
            return Direction::North;
    }
}

}  // namespace

SimulatedHardwareAdapter::SimulatedHardwareAdapter(std::vector<std::string> grid) : grid_(std::move(grid)) {
    if (grid_.empty()) {
        throw std::invalid_argument("scenario map must not be empty");
    }

    bool robotFound = false;
    for (int row = 0; row < static_cast<int>(grid_.size()); ++row) {
        for (int col = 0; col < static_cast<int>(grid_[row].size()); ++col) {
            const char cell = grid_[row][col];
            if (cell == 'R' || cell == '^' || cell == '>' || cell == 'v' || cell == '<') {
                if (robotFound) {
                    throw std::invalid_argument("scenario map must contain only one robot");
                }
                robotFound = true;
                robot_ = Position{row, col};
                direction_ = cell == 'R' ? Direction::North : directionFromMarker(cell);
                grid_[row][col] = '.';
            }
        }
    }

    if (!robotFound) {
        throw std::invalid_argument("scenario map must contain a robot marker");
    }
}

bool SimulatedHardwareAdapter::hasFrontObstacleInterrupt() const {
    return isObstacle(forwardPosition());
}

PeriodicSensorData SimulatedHardwareAdapter::readPeriodicSensors() const {
    return PeriodicSensorData{
        .leftObstacle = isObstacle(adjacent(robot_, turnLeft(direction_))),
        .dustDetected = hasDust(robot_),
    };
}

void SimulatedHardwareAdapter::applyCommand(const Command& command) {
    cleanCurrentCell(command);
    applyMotion(command.motion);
}

std::string SimulatedHardwareAdapter::render() const {
    std::vector<std::string> copy = grid_;
    if (robot_.row >= 0 && robot_.row < static_cast<int>(copy.size()) && robot_.col >= 0 &&
        robot_.col < static_cast<int>(copy[robot_.row].size())) {
        copy[robot_.row][robot_.col] = directionMarker(direction_);
    }

    std::ostringstream output;
    for (const auto& line : copy) {
        output << line << '\n';
    }
    return output.str();
}

int SimulatedHardwareAdapter::dustCleaned() const {
    return dustCleaned_;
}

int SimulatedHardwareAdapter::remainingDust() const {
    int count = 0;
    for (const auto& row : grid_) {
        count += static_cast<int>(std::count(row.begin(), row.end(), '*'));
    }
    return count;
}

Position SimulatedHardwareAdapter::robotPosition() const {
    return robot_;
}

Direction SimulatedHardwareAdapter::robotDirection() const {
    return direction_;
}

bool SimulatedHardwareAdapter::isObstacle(Position position) const {
    if (position.row < 0 || position.row >= static_cast<int>(grid_.size())) {
        return true;
    }

    if (position.col < 0 || position.col >= static_cast<int>(grid_[position.row].size())) {
        return true;
    }

    return grid_[position.row][position.col] == '#';
}

bool SimulatedHardwareAdapter::hasDust(Position position) const {
    if (position.row < 0 || position.row >= static_cast<int>(grid_.size())) {
        return false;
    }

    if (position.col < 0 || position.col >= static_cast<int>(grid_[position.row].size())) {
        return false;
    }

    return grid_[position.row][position.col] == '*';
}

Position SimulatedHardwareAdapter::adjacent(Position origin, Direction direction) const {
    switch (direction) {
        case Direction::North:
            return Position{origin.row - 1, origin.col};
        case Direction::East:
            return Position{origin.row, origin.col + 1};
        case Direction::South:
            return Position{origin.row + 1, origin.col};
        case Direction::West:
            return Position{origin.row, origin.col - 1};
    }
    return origin;
}

Position SimulatedHardwareAdapter::forwardPosition() const {
    return adjacent(robot_, direction_);
}

Position SimulatedHardwareAdapter::backwardPosition() const {
    return adjacent(robot_, opposite(direction_));
}

void SimulatedHardwareAdapter::cleanCurrentCell(const Command& command) {
    if (command.cleaningPower == CleaningPower::Off || !hasDust(robot_)) {
        return;
    }

    grid_[robot_.row][robot_.col] = '.';
    ++dustCleaned_;
}

void SimulatedHardwareAdapter::applyMotion(Motion motion) {
    switch (motion) {
        case Motion::Forward: {
            const Position target = forwardPosition();
            if (!isObstacle(target)) {
                robot_ = target;
            }
            break;
        }
        case Motion::Backward: {
            const Position target = backwardPosition();
            if (!isObstacle(target)) {
                robot_ = target;
            }
            break;
        }
        case Motion::TurnLeft:
            direction_ = turnLeft(direction_);
            break;
        case Motion::TurnRight:
            direction_ = turnRight(direction_);
            break;
        case Motion::None:
        case Motion::Stop:
            break;
    }
}

}  // namespace rvc
