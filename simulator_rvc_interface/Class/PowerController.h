#pragma once

class EventBus;

class PowerController {
public:
    explicit PowerController(EventBus* bus);

    void turnOn();
    void turnOff();

    [[nodiscard]] bool isPowerOn() const;

private:
    EventBus* bus_;
    bool powerOn_{false};
};
