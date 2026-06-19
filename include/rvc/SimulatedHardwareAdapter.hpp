#pragma once

#include "rvc/RvcHardwareAdapter.hpp"

#include <string>
#include <vector>

namespace rvc {

class SimulatedHardwareAdapter : public RvcHardwareAdapter {
public:
    explicit SimulatedHardwareAdapter(std::vector<std::string> grid);

    [[nodiscard]] bool hasFrontObstacleInterrupt() const override;
    [[nodiscard]] bool hasBackwardObstacleInterrupt() const override;
    [[nodiscard]] PeriodicSensorData readPeriodicSensors() const override;
    void applyCommand(const Command& command) override;

    [[nodiscard]] std::string render() const;
    [[nodiscard]] int dustCleaned() const;
    [[nodiscard]] int remainingDust() const;
    [[nodiscard]] Position robotPosition() const;
    [[nodiscard]] Direction robotDirection() const;

private:
    [[nodiscard]] bool isObstacle(Position position) const;
    [[nodiscard]] bool hasDust(Position position) const;
    [[nodiscard]] Position adjacent(Position origin, Direction direction) const;
    [[nodiscard]] Position forwardPosition() const;
    [[nodiscard]] Position backwardPosition() const;

    void cleanCurrentCell(const Command& command);
    void applyMotion(Motion motion);

    std::vector<std::string> grid_;
    Position robot_{};
    Direction direction_{Direction::North};
    int dustCleaned_{0};
};

}  // namespace rvc
