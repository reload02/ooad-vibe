#pragma once

#include "rvc/RvcController.hpp"
#include "rvc/RvcHardwareAdapter.hpp"

#include <memory>

namespace rvc {

class Rvc {
public:
    explicit Rvc(std::unique_ptr<RvcHardwareAdapter> hardwareAdapter, ControllerConfig config = {});

    void startCleaning();
    void stopCleaning();
    [[nodiscard]] Command tick();

    [[nodiscard]] bool lastFrontObstacleInterrupt() const;
    [[nodiscard]] const PeriodicSensorData& lastPeriodicSensors() const;
    [[nodiscard]] const Command& lastCommand() const;

private:
    RvcController controller_;
    std::unique_ptr<RvcHardwareAdapter> hardwareAdapter_;
    bool lastFrontObstacleInterrupt_{false};
    PeriodicSensorData lastPeriodicSensors_{};
    Command lastCommand_{};
};

}  // namespace rvc
