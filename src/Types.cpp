#include "rvc/Types.hpp"

namespace rvc {

Direction turnLeft(Direction direction) {
    switch (direction) {
        case Direction::North:
            return Direction::West;
        case Direction::West:
            return Direction::South;
        case Direction::South:
            return Direction::East;
        case Direction::East:
            return Direction::North;
    }
    return Direction::North;
}

Direction turnRight(Direction direction) {
    switch (direction) {
        case Direction::North:
            return Direction::East;
        case Direction::East:
            return Direction::South;
        case Direction::South:
            return Direction::West;
        case Direction::West:
            return Direction::North;
    }
    return Direction::North;
}

Direction opposite(Direction direction) {
    return turnRight(turnRight(direction));
}

std::string toString(Direction direction) {
    switch (direction) {
        case Direction::North:
            return "North";
        case Direction::East:
            return "East";
        case Direction::South:
            return "South";
        case Direction::West:
            return "West";
    }
    return "Unknown";
}

std::string toString(Motion motion) {
    switch (motion) {
        case Motion::None:
            return "None";
        case Motion::Stop:
            return "Stop";
        case Motion::Forward:
            return "Forward";
        case Motion::Backward:
            return "Backward";
        case Motion::TurnLeft:
            return "TurnLeft";
        case Motion::TurnRight:
            return "TurnRight";
    }
    return "Unknown";
}

std::string toString(CleaningPower power) {
    switch (power) {
        case CleaningPower::Off:
            return "Off";
        case CleaningPower::Normal:
            return "Normal";
        case CleaningPower::Boost:
            return "Boost";
    }
    return "Unknown";
}

std::string toString(ControllerState state) {
    switch (state) {
        case ControllerState::Idle:
            return "Idle";
        case ControllerState::Cleaning:
            return "Cleaning";
        case ControllerState::Avoiding:
            return "Avoiding";
        case ControllerState::RightProbing:
            return "RightProbing";
        case ControllerState::EscapeAligning:
            return "EscapeAligning";
        case ControllerState::Escaping:
            return "Escaping";
    }
    return "Unknown";
}

std::string toString(RightProbeState state) {
    switch (state) {
        case RightProbeState::None:
            return "None";
        case RightProbeState::Checking:
            return "Checking";
        case RightProbeState::Open:
            return "Open";
        case RightProbeState::Blocked:
            return "Blocked";
    }
    return "Unknown";
}

}  // namespace rvc
