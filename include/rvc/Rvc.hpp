#pragma once

#include "rvc/RvcController.hpp"

namespace rvc {

class Rvc {
public:
    explicit Rvc(ControllerConfig config = {});

    void startCleaning();
    void stopCleaning();
    void onFrontObstacleInterrupt();

    [[nodiscard]] Command tick(const PeriodicSensorData& periodicSensors);
    [[nodiscard]] SensorSnapshot readPeriodicSensors(const PeriodicSensorData& periodicSensors) const;

    [[nodiscard]] ControllerState state() const;
    [[nodiscard]] bool isRunning() const;
    [[nodiscard]] int boostTicksRemaining() const;

private:
    RvcController controller_;
};

}  // namespace rvc
