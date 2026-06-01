#pragma once

#include "rvc/Types.hpp"

namespace rvc {

class RvcHardwareAdapter {
public:
    virtual ~RvcHardwareAdapter() = default;

    [[nodiscard]] virtual bool hasFrontObstacleInterrupt() const = 0;
    [[nodiscard]] virtual PeriodicSensorData readPeriodicSensors() const = 0;
    virtual void applyCommand(const Command& command) = 0;
};

}  // namespace rvc
