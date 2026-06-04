#include "rvc/RvcController.hpp"

#include <gtest/gtest.h>

namespace {

using rvc::CleaningPower;
using rvc::Command;
using rvc::ControllerConfig;
using rvc::ControllerState;
using rvc::Motion;
using rvc::PeriodicSensorData;
using rvc::RightProbeState;
using rvc::RvcController;

TEST(RvcControllerTest, ControllerMovesForwardWhenPathIsClear) {
    RvcController controller;
    controller.startCleaning();

    const Command command = controller.tick(PeriodicSensorData{});

    EXPECT_EQ(command.motion, Motion::Forward);
    EXPECT_EQ(command.cleaningPower, CleaningPower::Normal);
    EXPECT_EQ(controller.state(), ControllerState::Cleaning);
    EXPECT_EQ(controller.rightProbeState(), RightProbeState::None);
}

TEST(RvcControllerTest, IdleControllerReturnsStopAndIgnoresFrontInterrupt) {
    RvcController controller;

    controller.onFrontObstacleInterrupt();
    const Command idleCommand = controller.tick(PeriodicSensorData{
        .leftObstacle = false,
        .dustDetected = true,
    });

    EXPECT_EQ(idleCommand.motion, Motion::Stop);
    EXPECT_EQ(idleCommand.cleaningPower, CleaningPower::Off);
    EXPECT_EQ(controller.state(), ControllerState::Idle);
    EXPECT_EQ(controller.rightProbeState(), RightProbeState::None);
    EXPECT_FALSE(controller.isRunning());

    controller.startCleaning();
    const Command runningCommand = controller.tick(PeriodicSensorData{});

    EXPECT_EQ(runningCommand.motion, Motion::Forward);
    EXPECT_EQ(runningCommand.cleaningPower, CleaningPower::Normal);
    EXPECT_EQ(controller.state(), ControllerState::Cleaning);
}

TEST(RvcControllerTest, StopCleaningReturnsStopAndOff) {
    RvcController controller;
    controller.startCleaning();
    (void)controller.tick(PeriodicSensorData{
        .leftObstacle = false,
        .dustDetected = true,
    });

    controller.stopCleaning();
    const Command command = controller.tick(PeriodicSensorData{
        .leftObstacle = false,
        .dustDetected = true,
    });

    EXPECT_EQ(command.motion, Motion::Stop);
    EXPECT_EQ(command.cleaningPower, CleaningPower::Off);
    EXPECT_EQ(controller.state(), ControllerState::Idle);
    EXPECT_EQ(controller.rightProbeState(), RightProbeState::None);
    EXPECT_FALSE(controller.isRunning());
    EXPECT_EQ(controller.boostTicksRemaining(), 0);
}

TEST(RvcControllerTest, FrontInterruptTriggersImmediateAvoidanceWhenLeftIsOpen) {
    RvcController controller;
    controller.startCleaning();
    controller.onFrontObstacleInterrupt();

    const Command command = controller.tick(PeriodicSensorData{
        .leftObstacle = false,
        .dustDetected = false,
    });

    EXPECT_EQ(command.motion, Motion::TurnLeft);
    EXPECT_EQ(command.cleaningPower, CleaningPower::Off);
    EXPECT_EQ(controller.state(), ControllerState::Avoiding);
    EXPECT_EQ(controller.rightProbeState(), RightProbeState::None);
}

TEST(RvcControllerTest, FrontInterruptIsConsumedAfterOneTick) {
    RvcController controller;
    controller.startCleaning();
    controller.onFrontObstacleInterrupt();

    const Command avoid = controller.tick(PeriodicSensorData{
        .leftObstacle = false,
        .dustDetected = false,
    });
    const Command resumed = controller.tick(PeriodicSensorData{
        .leftObstacle = false,
        .dustDetected = false,
    });

    EXPECT_EQ(avoid.motion, Motion::TurnLeft);
    EXPECT_EQ(avoid.cleaningPower, CleaningPower::Off);
    EXPECT_EQ(resumed.motion, Motion::Forward);
    EXPECT_EQ(resumed.cleaningPower, CleaningPower::Normal);
    EXPECT_EQ(controller.state(), ControllerState::Cleaning);
}

TEST(RvcControllerTest, RightProbeStartsWhenLeftBlocked) {
    RvcController controller;
    controller.startCleaning();
    controller.onFrontObstacleInterrupt();

    const Command command = controller.tick(PeriodicSensorData{
        .leftObstacle = true,
        .dustDetected = false,
    });

    EXPECT_EQ(command.motion, Motion::TurnRight);
    EXPECT_EQ(command.cleaningPower, CleaningPower::Off);
    EXPECT_EQ(controller.state(), ControllerState::RightProbing);
    EXPECT_EQ(controller.rightProbeState(), RightProbeState::Checking);
}

TEST(RvcControllerTest, RightProbeOpenResumesForward) {
    RvcController controller;
    controller.startCleaning();
    controller.onFrontObstacleInterrupt();

    const Command probe = controller.tick(PeriodicSensorData{
        .leftObstacle = true,
        .dustDetected = false,
    });
    const Command resumed = controller.tick(PeriodicSensorData{
        .leftObstacle = true,
        .dustDetected = false,
    });

    EXPECT_EQ(probe.motion, Motion::TurnRight);
    EXPECT_EQ(resumed.motion, Motion::Forward);
    EXPECT_EQ(resumed.cleaningPower, CleaningPower::Normal);
    EXPECT_EQ(controller.state(), ControllerState::Cleaning);
    EXPECT_EQ(controller.rightProbeState(), RightProbeState::Open);

    const Command nextClear = controller.tick(PeriodicSensorData{});
    EXPECT_EQ(nextClear.motion, Motion::Forward);
    EXPECT_EQ(controller.rightProbeState(), RightProbeState::None);
}

TEST(RvcControllerTest, RightProbeBlockedAlignsBeforeEscaping) {
    RvcController controller;
    controller.startCleaning();
    controller.onFrontObstacleInterrupt();

    const Command probe = controller.tick(PeriodicSensorData{
        .leftObstacle = true,
        .dustDetected = false,
    });
    controller.onFrontObstacleInterrupt();
    const Command align = controller.tick(PeriodicSensorData{
        .leftObstacle = true,
        .dustDetected = false,
    });
    const Command backward = controller.tick(PeriodicSensorData{
        .leftObstacle = true,
        .dustDetected = false,
    });

    EXPECT_EQ(probe.motion, Motion::TurnRight);
    EXPECT_EQ(align.motion, Motion::TurnLeft);
    EXPECT_EQ(align.cleaningPower, CleaningPower::Off);
    EXPECT_EQ(backward.motion, Motion::Backward);
    EXPECT_EQ(backward.cleaningPower, CleaningPower::Off);
    EXPECT_EQ(controller.state(), ControllerState::Escaping);
    EXPECT_EQ(controller.rightProbeState(), RightProbeState::Blocked);
}

TEST(RvcControllerTest, EscapingBacksUpThenReprobesRight) {
    RvcController controller;
    controller.startCleaning();
    controller.onFrontObstacleInterrupt();

    (void)controller.tick(PeriodicSensorData{
        .leftObstacle = true,
        .dustDetected = false,
    });
    controller.onFrontObstacleInterrupt();
    (void)controller.tick(PeriodicSensorData{
        .leftObstacle = true,
        .dustDetected = false,
    });
    const Command backward = controller.tick(PeriodicSensorData{
        .leftObstacle = true,
        .dustDetected = false,
    });
    const Command reprobe = controller.tick(PeriodicSensorData{
        .leftObstacle = true,
        .dustDetected = false,
    });

    EXPECT_EQ(backward.motion, Motion::Backward);
    EXPECT_EQ(reprobe.motion, Motion::TurnRight);
    EXPECT_EQ(reprobe.cleaningPower, CleaningPower::Off);
    EXPECT_EQ(controller.state(), ControllerState::RightProbing);
    EXPECT_EQ(controller.rightProbeState(), RightProbeState::Checking);
}

TEST(RvcControllerTest, EscapingTurnsLeftWhenLeftOpens) {
    RvcController controller;
    controller.startCleaning();
    controller.onFrontObstacleInterrupt();

    (void)controller.tick(PeriodicSensorData{
        .leftObstacle = true,
        .dustDetected = false,
    });
    controller.onFrontObstacleInterrupt();
    (void)controller.tick(PeriodicSensorData{
        .leftObstacle = true,
        .dustDetected = false,
    });
    (void)controller.tick(PeriodicSensorData{
        .leftObstacle = true,
        .dustDetected = false,
    });
    const Command sideExit = controller.tick(PeriodicSensorData{
        .leftObstacle = false,
        .dustDetected = false,
    });

    EXPECT_EQ(sideExit.motion, Motion::TurnLeft);
    EXPECT_EQ(sideExit.cleaningPower, CleaningPower::Off);
    EXPECT_EQ(controller.state(), ControllerState::Avoiding);
    EXPECT_EQ(controller.rightProbeState(), RightProbeState::None);
}

TEST(RvcControllerTest, RightProbeWithDustKeepsCleanerOffButPreservesBoostBudget) {
    RvcController controller(ControllerConfig{.dustBoostTicks = 3});
    controller.startCleaning();
    controller.onFrontObstacleInterrupt();

    const Command probe = controller.tick(PeriodicSensorData{
        .leftObstacle = true,
        .dustDetected = true,
    });
    const int remainingAfterProbeDust = controller.boostTicksRemaining();
    const Command resumed = controller.tick(PeriodicSensorData{
        .leftObstacle = true,
        .dustDetected = false,
    });

    EXPECT_EQ(probe.motion, Motion::TurnRight);
    EXPECT_EQ(probe.cleaningPower, CleaningPower::Off);
    EXPECT_EQ(remainingAfterProbeDust, 3);
    EXPECT_EQ(resumed.motion, Motion::Forward);
    EXPECT_EQ(resumed.cleaningPower, CleaningPower::Boost);
    EXPECT_EQ(controller.boostTicksRemaining(), 2);
}

TEST(RvcControllerTest, DustBoostLastsConfiguredTicks) {
    RvcController controller(ControllerConfig{.dustBoostTicks = 3});
    controller.startCleaning();

    const Command dustTick = controller.tick(PeriodicSensorData{
        .leftObstacle = false,
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

TEST(RvcControllerTest, DustDetectionRefreshesBoostBudget) {
    RvcController controller(ControllerConfig{.dustBoostTicks = 3});
    controller.startCleaning();

    const Command firstDust = controller.tick(PeriodicSensorData{
        .leftObstacle = false,
        .dustDetected = true,
    });
    const Command agingBoost = controller.tick(PeriodicSensorData{
        .leftObstacle = false,
        .dustDetected = false,
    });
    const int agedRemaining = controller.boostTicksRemaining();
    const Command refreshedDust = controller.tick(PeriodicSensorData{
        .leftObstacle = false,
        .dustDetected = true,
    });
    const int refreshedRemaining = controller.boostTicksRemaining();

    EXPECT_EQ(firstDust.cleaningPower, CleaningPower::Boost);
    EXPECT_EQ(agingBoost.cleaningPower, CleaningPower::Boost);
    EXPECT_EQ(agedRemaining, 2);
    EXPECT_EQ(refreshedDust.cleaningPower, CleaningPower::Boost);
    EXPECT_EQ(refreshedRemaining, 3);
}

TEST(RvcControllerTest, ZeroTickBoostConfigurationDoesNotBoostOnDust) {
    RvcController controller(ControllerConfig{.dustBoostTicks = 0});
    controller.startCleaning();

    const Command command = controller.tick(PeriodicSensorData{
        .leftObstacle = false,
        .dustDetected = true,
    });

    EXPECT_EQ(command.motion, Motion::Forward);
    EXPECT_EQ(command.cleaningPower, CleaningPower::Normal);
    EXPECT_EQ(controller.boostTicksRemaining(), 0);
}

TEST(RvcControllerTest, AvoidanceOutputStaysOffWhileBoostStateIsMaintained) {
    RvcController controller(ControllerConfig{.dustBoostTicks = 4});
    controller.startCleaning();

    const Command dustTick = controller.tick(PeriodicSensorData{
        .leftObstacle = false,
        .dustDetected = true,
    });

    controller.onFrontObstacleInterrupt();
    const Command avoidTick = controller.tick(PeriodicSensorData{
        .leftObstacle = false,
        .dustDetected = false,
    });
    const Command resumedTick = controller.tick(PeriodicSensorData{
        .leftObstacle = false,
        .dustDetected = false,
    });

    EXPECT_EQ(dustTick.motion, Motion::Forward);
    EXPECT_EQ(dustTick.cleaningPower, CleaningPower::Boost);
    EXPECT_EQ(avoidTick.motion, Motion::TurnLeft);
    EXPECT_EQ(avoidTick.cleaningPower, CleaningPower::Off);
    EXPECT_EQ(resumedTick.motion, Motion::Forward);
    EXPECT_EQ(resumedTick.cleaningPower, CleaningPower::Boost);
}

}  // namespace
