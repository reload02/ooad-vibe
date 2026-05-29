#include "rvc/Rvc.hpp"
#include "rvc/RvcSubsystems.hpp"

#include <gtest/gtest.h>

namespace {

using rvc::CleaningPolicy;
using rvc::CleaningPower;
using rvc::Command;
using rvc::CommandComposer;
using rvc::ControllerConfig;
using rvc::ControllerState;
using rvc::Motion;
using rvc::NavigationDecision;
using rvc::NavigationPolicy;
using rvc::PeriodicSensorData;
using rvc::Rvc;
using rvc::SensorFusion;
using rvc::SensorSnapshot;

TEST(RvcSubsystemTest, SensorFusionCombinesFrontInterruptAndPeriodicInputs) {
    SensorFusion fusion;
    fusion.recordFrontObstacleInterrupt();

    const SensorSnapshot snapshot = fusion.fuse(PeriodicSensorData{
        .leftObstacle = true,
        .rightObstacle = false,
        .dustDetected = true,
    });

    EXPECT_TRUE(snapshot.frontObstacle);
    EXPECT_TRUE(snapshot.leftObstacle);
    EXPECT_FALSE(snapshot.rightObstacle);
    EXPECT_TRUE(snapshot.dustDetected);

    fusion.clearFrontObstacleInterrupt();
    EXPECT_FALSE(fusion.fuse(PeriodicSensorData{}).frontObstacle);
}

TEST(RvcSubsystemTest, NavigationPolicyChoosesForwardAvoidanceAlternationAndEscape) {
    NavigationPolicy navigation;
    navigation.startCleaning();

    const NavigationDecision clear = navigation.decide(SensorSnapshot{});
    EXPECT_EQ(clear.motion, Motion::Forward);
    EXPECT_EQ(navigation.state(), ControllerState::Cleaning);

    const NavigationDecision left = navigation.decide(SensorSnapshot{
        .frontObstacle = true,
        .leftObstacle = false,
        .rightObstacle = true,
        .dustDetected = false,
    });
    EXPECT_EQ(left.motion, Motion::TurnLeft);
    EXPECT_EQ(navigation.state(), ControllerState::Avoiding);

    const NavigationDecision firstOpenChoice = navigation.decide(SensorSnapshot{
        .frontObstacle = true,
        .leftObstacle = false,
        .rightObstacle = false,
        .dustDetected = false,
    });
    const NavigationDecision secondOpenChoice = navigation.decide(SensorSnapshot{
        .frontObstacle = true,
        .leftObstacle = false,
        .rightObstacle = false,
        .dustDetected = false,
    });
    EXPECT_EQ(firstOpenChoice.motion, Motion::TurnLeft);
    EXPECT_EQ(secondOpenChoice.motion, Motion::TurnRight);

    const NavigationDecision blocked = navigation.decide(SensorSnapshot{
        .frontObstacle = true,
        .leftObstacle = true,
        .rightObstacle = true,
        .dustDetected = false,
    });
    const NavigationDecision stillBlocked = navigation.decide(SensorSnapshot{
        .frontObstacle = false,
        .leftObstacle = true,
        .rightObstacle = true,
        .dustDetected = false,
    });
    const NavigationDecision sideExit = navigation.decide(SensorSnapshot{
        .frontObstacle = false,
        .leftObstacle = true,
        .rightObstacle = false,
        .dustDetected = false,
    });

    EXPECT_EQ(blocked.motion, Motion::Backward);
    EXPECT_EQ(stillBlocked.motion, Motion::Backward);
    EXPECT_EQ(sideExit.motion, Motion::TurnRight);
    EXPECT_EQ(navigation.state(), ControllerState::Avoiding);
}

TEST(RvcSubsystemTest, CleaningPolicyMaintainsBoostBudget) {
    CleaningPolicy cleaning(ControllerConfig{.dustBoostTicks = 2});

    EXPECT_EQ(cleaning.update(true), CleaningPower::Boost);
    EXPECT_EQ(cleaning.boostTicksRemaining(), 2);
    EXPECT_EQ(cleaning.update(false), CleaningPower::Boost);
    EXPECT_EQ(cleaning.boostTicksRemaining(), 1);
    EXPECT_EQ(cleaning.update(false), CleaningPower::Normal);
    EXPECT_EQ(cleaning.boostTicksRemaining(), 0);

    EXPECT_EQ(cleaning.update(true), CleaningPower::Boost);
    cleaning.reset();
    EXPECT_EQ(cleaning.boostTicksRemaining(), 0);
}

TEST(RvcSubsystemTest, CommandComposerAllowsCleaningOnlyWhileMovingForward) {
    CommandComposer composer;

    const Command forward = composer.compose(Motion::Forward, CleaningPower::Boost, "forward");
    const Command turn = composer.compose(Motion::TurnLeft, CleaningPower::Boost, "turn");
    const Command backward = composer.compose(Motion::Backward, CleaningPower::Normal, "backward");
    const Command stop = composer.compose(Motion::Stop, CleaningPower::Normal, "stop");

    EXPECT_EQ(forward.cleaningPower, CleaningPower::Boost);
    EXPECT_EQ(turn.cleaningPower, CleaningPower::Off);
    EXPECT_EQ(backward.cleaningPower, CleaningPower::Off);
    EXPECT_EQ(stop.cleaningPower, CleaningPower::Off);
}

TEST(RvcFacadeTest, FacadeReturnsSameCommandsAsControllerContract) {
    Rvc rvc(ControllerConfig{.dustBoostTicks = 3});

    const Command idle = rvc.tick(PeriodicSensorData{.dustDetected = true});
    EXPECT_EQ(idle.motion, Motion::Stop);
    EXPECT_EQ(idle.cleaningPower, CleaningPower::Off);

    rvc.startCleaning();
    const Command clear = rvc.tick(PeriodicSensorData{});
    EXPECT_EQ(clear.motion, Motion::Forward);
    EXPECT_EQ(clear.cleaningPower, CleaningPower::Normal);

    rvc.onFrontObstacleInterrupt();
    const Command avoid = rvc.tick(PeriodicSensorData{
        .leftObstacle = false,
        .rightObstacle = true,
        .dustDetected = true,
    });
    EXPECT_EQ(avoid.motion, Motion::TurnLeft);
    EXPECT_EQ(avoid.cleaningPower, CleaningPower::Off);
    EXPECT_EQ(rvc.boostTicksRemaining(), 3);
}

}  // namespace
