#include "rvc/RvcController.hpp"

#include <gtest/gtest.h>

namespace {

using rvc::CleaningPower;
using rvc::Command;
using rvc::ControllerConfig;
using rvc::ControllerState;
using rvc::Motion;
using rvc::PeriodicSensorData;
using rvc::RvcController;

TEST(RvcControllerTest, ControllerMovesForwardWhenPathIsClear) {
    RvcController controller;
    controller.startCleaning();

    const Command command = controller.tick(PeriodicSensorData{});

    EXPECT_EQ(command.motion, Motion::Forward);
    EXPECT_EQ(command.cleaningPower, CleaningPower::Normal);
    EXPECT_EQ(controller.state(), ControllerState::Cleaning);
}

TEST(RvcControllerTest, FrontInterruptTriggersImmediateAvoidance) {
    RvcController controller;
    controller.startCleaning();
    controller.onFrontObstacleInterrupt();

    const Command command = controller.tick(PeriodicSensorData{
        .leftObstacle = false,
        .rightObstacle = true,
        .dustDetected = false,
    });

    EXPECT_EQ(command.motion, Motion::TurnLeft);
    EXPECT_EQ(command.cleaningPower, CleaningPower::Normal);
    EXPECT_EQ(controller.state(), ControllerState::Avoiding);
}

TEST(RvcControllerTest, TurnsTowardOpenSide) {
    RvcController controller;
    controller.startCleaning();
    controller.onFrontObstacleInterrupt();

    const Command command = controller.tick(PeriodicSensorData{
        .leftObstacle = true,
        .rightObstacle = false,
        .dustDetected = false,
    });

    EXPECT_EQ(command.motion, Motion::TurnRight);
    EXPECT_EQ(controller.state(), ControllerState::Avoiding);
}

TEST(RvcControllerTest, AlternatesWhenBothSidesAreOpen) {
    RvcController controller;
    controller.startCleaning();

    controller.onFrontObstacleInterrupt();
    const Command first = controller.tick(PeriodicSensorData{
        .leftObstacle = false,
        .rightObstacle = false,
        .dustDetected = false,
    });

    controller.onFrontObstacleInterrupt();
    const Command second = controller.tick(PeriodicSensorData{
        .leftObstacle = false,
        .rightObstacle = false,
        .dustDetected = false,
    });

    EXPECT_EQ(first.motion, Motion::TurnLeft);
    EXPECT_EQ(second.motion, Motion::TurnRight);
}

TEST(RvcControllerTest, AllBlockedEntersEscapingAndKeepsBackingUp) {
    RvcController controller;
    controller.startCleaning();
    controller.onFrontObstacleInterrupt();

    const Command first = controller.tick(PeriodicSensorData{
        .leftObstacle = true,
        .rightObstacle = true,
        .dustDetected = false,
    });

    controller.onFrontObstacleInterrupt();
    const Command second = controller.tick(PeriodicSensorData{
        .leftObstacle = true,
        .rightObstacle = true,
        .dustDetected = false,
    });

    EXPECT_EQ(first.motion, Motion::Backward);
    EXPECT_EQ(second.motion, Motion::Backward);
    EXPECT_EQ(controller.state(), ControllerState::Escaping);
}

TEST(RvcControllerTest, EscapingIgnoresOpenFrontUntilSideOpens) {
    RvcController controller;
    controller.startCleaning();
    controller.onFrontObstacleInterrupt();

    (void)controller.tick(PeriodicSensorData{
        .leftObstacle = true,
        .rightObstacle = true,
        .dustDetected = false,
    });

    const Command stillEscaping = controller.tick(PeriodicSensorData{
        .leftObstacle = true,
        .rightObstacle = true,
        .dustDetected = false,
    });

    EXPECT_EQ(stillEscaping.motion, Motion::Backward);
    EXPECT_EQ(controller.state(), ControllerState::Escaping);

    const Command sideExit = controller.tick(PeriodicSensorData{
        .leftObstacle = false,
        .rightObstacle = true,
        .dustDetected = false,
    });

    EXPECT_EQ(sideExit.motion, Motion::TurnLeft);
    EXPECT_EQ(controller.state(), ControllerState::Avoiding);
}

TEST(RvcControllerTest, DustBoostLastsConfiguredTicks) {
    RvcController controller(ControllerConfig{.dustBoostTicks = 3});
    controller.startCleaning();

    const Command dustTick = controller.tick(PeriodicSensorData{
        .leftObstacle = false,
        .rightObstacle = false,
        .dustDetected = true,
    });
    const Command boostTick2 = controller.tick(PeriodicSensorData{});
    const Command boostTick3 = controller.tick(PeriodicSensorData{});
    const Command normalTick = controller.tick(PeriodicSensorData{});

    EXPECT_EQ(dustTick.cleaningPower, CleaningPower::Boost);
    EXPECT_EQ(boostTick2.cleaningPower, CleaningPower::Boost);
    EXPECT_EQ(boostTick3.cleaningPower, CleaningPower::Boost);
    EXPECT_EQ(normalTick.cleaningPower, CleaningPower::Normal);
}

}  // namespace
