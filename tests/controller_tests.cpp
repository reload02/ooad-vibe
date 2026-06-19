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
    EXPECT_EQ(command.cleaningPower, CleaningPower::Normal); // R3: 회전 중에도 클리너 On
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
    EXPECT_EQ(avoid.cleaningPower, CleaningPower::Normal); // R3: 회전 중에도 클리너 On
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
    EXPECT_EQ(command.cleaningPower, CleaningPower::Normal); // R3: 회전 중에도 클리너 On
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
    EXPECT_EQ(probe.cleaningPower, CleaningPower::Normal); // R3: 온 유지
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
    EXPECT_EQ(align.cleaningPower, CleaningPower::Normal); // R3: 온 유지
    EXPECT_EQ(backward.motion, Motion::Backward);
    EXPECT_EQ(backward.cleaningPower, CleaningPower::Normal); // R3: 온 유지
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
    EXPECT_EQ(backward.cleaningPower, CleaningPower::Normal); // R3: 온 유지
    EXPECT_EQ(reprobe.motion, Motion::TurnRight);
    EXPECT_EQ(reprobe.cleaningPower, CleaningPower::Normal); // R3: 온 유지
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
    EXPECT_EQ(sideExit.cleaningPower, CleaningPower::Normal); // R3: 온 유지
    EXPECT_EQ(controller.state(), ControllerState::Avoiding);
    EXPECT_EQ(controller.rightProbeState(), RightProbeState::None);
}

TEST(RvcControllerTest, DustSpinBoostSequenceAndEscape) {
    RvcController controller;
    controller.startCleaning();

    // 1. 먼지 감지 -> 180도 회전 1틱째 (TurnRight / Boost)
    const Command tick1 = controller.tick(PeriodicSensorData{
        .leftObstacle = false,
        .dustDetected = true,
    });
    EXPECT_EQ(tick1.motion, Motion::TurnRight);
    EXPECT_EQ(tick1.cleaningPower, CleaningPower::Boost);
    EXPECT_EQ(controller.state(), ControllerState::DustSpinning);

    // 2. 180도 회전 2틱째 (TurnRight / Boost)
    const Command tick2 = controller.tick(PeriodicSensorData{
        .leftObstacle = false,
        .dustDetected = true, // 불소멸되어 먼지는 여전히 존재
    });
    EXPECT_EQ(tick2.motion, Motion::TurnRight);
    EXPECT_EQ(tick2.cleaningPower, CleaningPower::Boost);
    EXPECT_EQ(controller.state(), ControllerState::DustSpinning);

    // 3. 이탈 틱 (Backward / Normal / 먼지 감지 강제 무시)
    const Command tick3 = controller.tick(PeriodicSensorData{
        .leftObstacle = false,
        .dustDetected = true, // 먼지가 계속 감지되지만 무시해야 함
    });
    EXPECT_EQ(tick3.motion, Motion::Backward);
    EXPECT_EQ(tick3.cleaningPower, CleaningPower::Normal);
    EXPECT_EQ(controller.state(), ControllerState::DustLeaving);

    // 4. 원래 상태(Cleaning) 복귀 틱
    const Command tick4 = controller.tick(PeriodicSensorData{
        .leftObstacle = false,
        .dustDetected = false,
    });
    EXPECT_EQ(tick4.motion, Motion::Forward);
    EXPECT_EQ(tick4.cleaningPower, CleaningPower::Normal);
    EXPECT_EQ(controller.state(), ControllerState::Cleaning);
}

TEST(RvcControllerTest, EscapingStopsOnBackwardObstacleInterrupt) {
    RvcController controller;
    controller.startCleaning();
    controller.onFrontObstacleInterrupt();

    // Escaping 상태 유도
    (void)controller.tick(PeriodicSensorData{.leftObstacle = true});
    controller.onFrontObstacleInterrupt();
    (void)controller.tick(PeriodicSensorData{.leftObstacle = true}); // EscapeAligning

    // 후방 장애물 인터럽트 트리거
    controller.onBackwardObstacleInterrupt();
    const Command command = controller.tick(PeriodicSensorData{.leftObstacle = true});

    EXPECT_EQ(command.motion, Motion::Stop);
    EXPECT_EQ(command.cleaningPower, CleaningPower::Normal); // 클리너 온 유지
    EXPECT_EQ(controller.state(), ControllerState::Escaping);
}

}  // namespace
