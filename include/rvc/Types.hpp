#pragma once

#include <string>

namespace rvc {

enum class Direction {
    North,
    East,
    South,
    West,
};

enum class Motion {
    None,
    Stop,
    Forward,
    Backward,
    TurnLeft,
    TurnRight,
};

enum class CleaningPower {
    Off,
    Normal,
    Boost,
};

enum class ControllerState {
    Idle,
    Cleaning,
    Avoiding,
    RightProbing,
    EscapeAligning,
    Escaping,
    DustSpinning,
    DustLeaving,
    DustLeavingBackward,
    DustLeavingForward,
};

enum class RightProbeState {
    None,
    Checking,
    Open,
    Blocked,
};

struct Position {
    int row{0};
    int col{0};

    [[nodiscard]] bool operator==(const Position& other) const = default;
};

struct PeriodicSensorData {
    bool leftObstacle{false};
    bool dustDetected{false};
};

struct SensorSnapshot {
    bool frontObstacle{false};
    bool backwardObstacle{false};
    bool leftObstacle{false};
    RightProbeState rightProbe{RightProbeState::None};
    bool dustDetected{false};
};

struct Command {
    Motion motion{Motion::None};
    CleaningPower cleaningPower{CleaningPower::Off};
    std::string reason;
};

[[nodiscard]] Direction turnLeft(Direction direction);
[[nodiscard]] Direction turnRight(Direction direction);
[[nodiscard]] Direction opposite(Direction direction);
[[nodiscard]] std::string toString(Direction direction);
[[nodiscard]] std::string toString(Motion motion);
[[nodiscard]] std::string toString(CleaningPower power);
[[nodiscard]] std::string toString(ControllerState state);
[[nodiscard]] std::string toString(RightProbeState state);

}  // namespace rvc
