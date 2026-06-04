#include "rvc/CleaningPowerPolicy.hpp"
#include "rvc/NavigationPolicy.hpp"

#include <gtest/gtest.h>

namespace {

using rvc::CleaningPower;
using rvc::CleaningPowerPolicy;
using rvc::ControllerState;
using rvc::Motion;
using rvc::NavigationDecision;
using rvc::NavigationPolicy;
using rvc::RightProbeState;
using rvc::SensorSnapshot;

TEST(CleaningPowerPolicyTest, DustRefreshesAndAgesBoostBudget) {
    CleaningPowerPolicy policy(2);

    EXPECT_EQ(policy.update(true), CleaningPower::Boost);
    EXPECT_EQ(policy.boostTicksRemaining(), 2);

    EXPECT_EQ(policy.update(false), CleaningPower::Boost);
    EXPECT_EQ(policy.boostTicksRemaining(), 1);

    EXPECT_EQ(policy.update(false), CleaningPower::Normal);
    EXPECT_EQ(policy.boostTicksRemaining(), 0);
}

TEST(CleaningPowerPolicyTest, ResetClearsBoostBudget) {
    CleaningPowerPolicy policy(3);

    (void)policy.update(true);
    policy.reset();

    EXPECT_EQ(policy.boostTicksRemaining(), 0);
    EXPECT_EQ(policy.update(false), CleaningPower::Normal);
}

TEST(NavigationPolicyTest, FrontBlockedAndLeftOpenTurnsLeft) {
    NavigationPolicy policy;
    policy.startCleaning();

    const NavigationDecision decision = policy.decide(SensorSnapshot{
        .frontObstacle = true,
        .leftObstacle = false,
    });

    EXPECT_EQ(decision.motion, Motion::TurnLeft);
    EXPECT_EQ(policy.state(), ControllerState::Avoiding);
    EXPECT_EQ(policy.rightProbeState(), RightProbeState::None);
}

TEST(NavigationPolicyTest, IdlePolicyReturnsStop) {
    NavigationPolicy policy;

    const NavigationDecision decision = policy.decide(SensorSnapshot{});

    EXPECT_EQ(decision.motion, Motion::Stop);
    EXPECT_EQ(policy.state(), ControllerState::Idle);
    EXPECT_EQ(policy.rightProbeState(), RightProbeState::None);
}

TEST(NavigationPolicyTest, LeftBlockedProbesRightAndOpenProbeResumesForward) {
    NavigationPolicy policy;
    policy.startCleaning();

    const NavigationDecision probe = policy.decide(SensorSnapshot{
        .frontObstacle = true,
        .leftObstacle = true,
    });
    const NavigationDecision resumed = policy.decide(SensorSnapshot{
        .frontObstacle = false,
        .leftObstacle = true,
    });

    EXPECT_EQ(probe.motion, Motion::TurnRight);
    EXPECT_EQ(resumed.motion, Motion::Forward);
    EXPECT_EQ(policy.state(), ControllerState::Cleaning);
    EXPECT_EQ(policy.rightProbeState(), RightProbeState::Open);
}

TEST(NavigationPolicyTest, BlockedRightProbeRestoresHeadingBeforeBackingUp) {
    NavigationPolicy policy;
    policy.startCleaning();

    const NavigationDecision probe = policy.decide(SensorSnapshot{
        .frontObstacle = true,
        .leftObstacle = true,
    });
    const NavigationDecision align = policy.decide(SensorSnapshot{
        .frontObstacle = true,
        .leftObstacle = true,
    });
    const NavigationDecision backward = policy.decide(SensorSnapshot{
        .frontObstacle = false,
        .leftObstacle = true,
    });

    EXPECT_EQ(probe.motion, Motion::TurnRight);
    EXPECT_EQ(align.motion, Motion::TurnLeft);
    EXPECT_EQ(backward.motion, Motion::Backward);
    EXPECT_EQ(policy.state(), ControllerState::Escaping);
    EXPECT_EQ(policy.rightProbeState(), RightProbeState::Blocked);
}

}  // namespace
