#pragma once

#include "HDWARE/HwCleaner.h"
#include "rvc/RvcController.hpp"

class Timer;

class EventBus {
public:
    EventBus() = default;

    void attachCleaner(HwCleaner* cleaner, Timer* timer);

    void setPowerOn(bool powerOn);
    [[nodiscard]] bool isPowerOn() const;

    void publishStartCleaning();
    void publishStopCleaning();
    void publishFrontObstacleDetected();
    void updatePeriodicSensors(bool leftObstacle, bool dustDetected);

    [[nodiscard]] rvc::Command runControllerTick();
    [[nodiscard]] const rvc::Command& lastCommand() const;
    [[nodiscard]] rvc::ControllerState controllerState() const;
    [[nodiscard]] bool isControllerRunning() const;
    [[nodiscard]] bool isAvoiding() const;

private:
    void applyCleanerCommand(const rvc::Command& command);

    rvc::RvcController controller_;
    rvc::PeriodicSensorData periodicSensors_{};
    rvc::Command lastCommand_{};
    HwCleaner* cleaner_{nullptr};
    Timer* timer_{nullptr};
    bool powerOn_{false};
};
