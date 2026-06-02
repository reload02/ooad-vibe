#pragma once

#include "Class/CleanerController.h"
#include "Class/EventBus.h"
#include "Class/MotorController.h"
#include "Class/PowerController.h"
#include "Class/SensorController.h"
#include "Class/Timer.h"
#include "Simulator/GridMap.h"
#include "Simulator/SimulatedCleaner.h"
#include "Simulator/SimulatedMotor.h"
#include "Simulator/SimulatedSensor.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class RvcSimulator {
public:
    RvcSimulator();
    ~RvcSimulator();

    void run();
    void reset();
    void resetSystemTestMap();
    void resetRandomMap(uint32_t seed = 0);
    void turnOn();
    void turnOff();
    void step();
    void autoStep(int count);
    void forceFrontObstacle();
    bool addDust(int x, int y);
    bool toggleWall(int x, int y);
    std::string statusText() const;
    std::string render() const;

    Point getRobotPoint() const;
    Point getRobotDirection() const;
    bool isPowerOn() const;
    bool isCleanerOn() const;
    bool isPowerUp() const;
    bool isAvoiding() const;
    int getConsecutiveAvoidSteps() const;
    bool isWallAt(int x, int y) const;
    bool loadSystemTestScenario(const std::vector<std::string>& layoutRowsTopToBottom,
                                Point startPoint,
                                Point startDirection);
    void setSensorFault(SensorDirection direction, SimulatedSensor::FaultMode mode);
    void setMotorBroken(bool broken);
    bool isMotorBlocked() const;
    void setAutoStepSleepEnabled(bool enabled);
    void setStepTraceEnabled(bool enabled);
    void showCurrentScreen() const;

private:
    GridMap map;
    EventBus bus;
    SimulatedMotor motor;
    SimulatedSensor frontSensor;
    SimulatedSensor leftSensor;
    SimulatedSensor dustSensor;
    SimulatedCleaner cleaner;
    Timer cleanerTimer;
    SensorController sensorController;
    CleanerController cleanerController;
    MotorController motorController;
    PowerController powerController;
    bool powerOn;
    bool autoStepSleepEnabled;
    int consecutiveAvoidSteps;
    bool stepTraceEnabled;

    void printScreen() const;
    void printHelp() const;
    bool handleCommand(const std::string& line);
    std::string directionText() const;
    void runFreeMode();
};
