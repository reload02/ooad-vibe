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

TEST(CleaningPowerPolicyTest, BoostPowerOnlyDuringDustSpinningState) {
    CleaningPowerPolicy policy(3);

    EXPECT_EQ(policy.update(ControllerState::DustSpinning, true), CleaningPower::Boost);
    EXPECT_EQ(policy.update(ControllerState::DustSpinning, false), CleaningPower::Boost);

    EXPECT_EQ(policy.update(ControllerState::Cleaning, true), CleaningPower::Normal);
    EXPECT_EQ(policy.update(ControllerState::Escaping, false), CleaningPower::Normal);
    EXPECT_EQ(policy.update(ControllerState::DustLeaving, true), CleaningPower::Normal);
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

TEST(NavigationPolicyTest, DustSpinningAndLeavingSequence) {
    NavigationPolicy policy;
    policy.startCleaning();

    // 1. 먼지 조우 -> DustSpinning 돌입 (1번째 틱)
    NavigationDecision dec1 = policy.decide(SensorSnapshot{
        .dustDetected = true,
    });
    EXPECT_EQ(dec1.motion, Motion::TurnRight);
    EXPECT_EQ(policy.state(), ControllerState::DustSpinning);

    // 2. DustSpinning (2번째 틱) -> 2틱 완료 시점 (상태는 다음 틱 시작 시 DustLeaving으로 전이 예정)
    NavigationDecision dec2 = policy.decide(SensorSnapshot{});
    EXPECT_EQ(dec2.motion, Motion::TurnRight);
    EXPECT_EQ(policy.state(), ControllerState::DustSpinning);

    // 3. DustLeaving (3번째 틱) -> 대기 판단 상태로 이동 (DustLeavingBackward)
    NavigationDecision dec3 = policy.decide(SensorSnapshot{});
    EXPECT_EQ(dec3.motion, Motion::Stop);
    EXPECT_EQ(policy.state(), ControllerState::DustLeavingBackward);

    // 4. DustLeavingBackward (4번째 틱) -> Backward 수행하여 Escaping 상태로 이동
    NavigationDecision dec4 = policy.decide(SensorSnapshot{});
    EXPECT_EQ(dec4.motion, Motion::Backward);
    EXPECT_EQ(policy.state(), ControllerState::Escaping);

    // 5. Escaping (5번째 틱) -> leftObstacle = false 이므로 TurnLeft 및 Avoiding 상태로 이동
    NavigationDecision dec5 = policy.decide(SensorSnapshot{});
    EXPECT_EQ(dec5.motion, Motion::TurnLeft);
    EXPECT_EQ(policy.state(), ControllerState::Avoiding);

    // 6. Avoiding (6번째 틱) -> frontObstacle = false 이므로 Forward 및 Cleaning 상태 복구
    NavigationDecision dec6 = policy.decide(SensorSnapshot{});
    EXPECT_EQ(dec6.motion, Motion::Forward);
    EXPECT_EQ(policy.state(), ControllerState::Cleaning);
}

TEST(NavigationPolicyTest, EscapingStopsOnBackwardObstacle) {
    NavigationPolicy policy;
    policy.startCleaning();

    // Escaping 상태로 강제 전이시키기 위한 시나리오 진행
    (void)policy.decide(SensorSnapshot{.frontObstacle = true, .leftObstacle = true}); // RightProbing
    (void)policy.decide(SensorSnapshot{.frontObstacle = true, .leftObstacle = true}); // EscapeAligning
    NavigationDecision backward = policy.decide(SensorSnapshot{.frontObstacle = false, .leftObstacle = true}); // Escaping 돌입
    EXPECT_EQ(backward.motion, Motion::Backward);
    EXPECT_EQ(policy.state(), ControllerState::Escaping);

    // Escaping 상태에서 후방 장애물 감지 시 Stop 동작 검증
    NavigationDecision stopped = policy.decide(SensorSnapshot{.backwardObstacle = true, .leftObstacle = true});
    EXPECT_EQ(stopped.motion, Motion::Stop);
    EXPECT_EQ(policy.state(), ControllerState::Escaping);
}

}  // namespace
