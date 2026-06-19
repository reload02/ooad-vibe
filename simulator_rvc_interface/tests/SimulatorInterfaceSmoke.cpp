#include "Class/CleanerController.h"
#include "Class/EventBus.h"
#include "Class/MotorController.h"
#include "Class/PowerController.h"
#include "Class/SensorController.h"
#include "Class/Timer.h"
#include "HDWARE/HwCleaner.h"
#include "HDWARE/Motor.h"

namespace {

class FakeSensor final : public ISensor {
public:
    bool value{false};

    bool detect() override {
        return value;
    }
};

class FakeMotor final : public Motor {
public:
    int forwardCount{0};
    int backwardCount{0};
    int leftCount{0};
    int rightCount{0};

    void moveForward() override {
        ++forwardCount;
    }

    void moveBackward() override {
        ++backwardCount;
    }

    void turnLeft() override {
        ++leftCount;
    }

    void turnRight() override {
        ++rightCount;
    }
};

class FakeCleaner final : public HwCleaner {
public:
    bool turnedOn{false};
    bool boosted{false};

    void powerUp() override {
        if (turnedOn) {
            boosted = true;
        }
    }

    void powerRestore() override {
        boosted = false;
    }

    void turnOn() override {
        turnedOn = true;
    }

    void turnOff() override {
        turnedOn = false;
        boosted = false;
    }
};

int fail(int code) {
    return code;
}

}  // namespace

int main() {
    EventBus bus;
    FakeSensor leftSensor;
    FakeSensor dustSensor;
    FakeMotor motor;
    FakeCleaner cleaner;
    Timer timer;

    SensorController sensorController(&bus, &leftSensor, &dustSensor);
    CleanerController cleanerController(&bus, &cleaner, &timer);
    MotorController motorController(&bus, motor);
    PowerController powerController(&bus);

    sensorController.turnOn();
    motorController.turnOn();
    powerController.turnOn();
    sensorController.ChecknPowerUp();
    bus.publishStartCleaning();

    motorController.MCMove();
    if (motor.forwardCount != 1 || !cleaner.turnedOn || cleaner.boosted) {
        return fail(1);
    }

    sensorController.FrontObstacleDetected();
    sensorController.ChecknPowerUp();
    motorController.MCMove();
    if (motor.leftCount != 1 || !cleaner.turnedOn) {
        return fail(2);
    }

    dustSensor.value = true;
    sensorController.ChecknPowerUp();

    // R3 먼지 감지 틱 1: 180도 회전 1틱째 (TurnRight / Boost)
    motorController.MCMove();
    if (motor.rightCount != 1 || motor.forwardCount != 1 || !cleaner.turnedOn || !cleaner.boosted) {
        return fail(3);
    }

    // R3 먼지 감지 틱 2: 180도 회전 2틱째 (TurnRight / Boost)
    motorController.MCMove();
    if (motor.rightCount != 2 || motor.forwardCount != 1 || !cleaner.turnedOn || !cleaner.boosted) {
        return fail(5);
    }

    // R3 먼지 감지 틱 3: 이탈 틱 (Forward / Normal)
    motorController.MCMove();
    if (motor.forwardCount != 2 || !cleaner.turnedOn || cleaner.boosted) {
        return fail(6);
    }

    powerController.turnOff();
    if (bus.isControllerRunning() || cleaner.turnedOn || cleaner.boosted) {
        return fail(4);
    }

    return 0;
}
