#include "rvc/NavigationPolicy.hpp"

namespace rvc {

void NavigationPolicy::startCleaning() {
    state_ = ControllerState::Cleaning;
    rightProbe_ = RightProbeState::None;
}

void NavigationPolicy::stopCleaning() {
    state_ = ControllerState::Idle;
    rightProbe_ = RightProbeState::None;
}

NavigationDecision NavigationPolicy::decide(const SensorSnapshot& snapshot) {
    if (state_ == ControllerState::Idle) {
        return {
            .motion = Motion::Stop,
            .reason = "navigation idle",
        };
    }

    if (state_ == ControllerState::RightProbing) {
        if (snapshot.frontObstacle) {
            state_ = ControllerState::EscapeAligning;
            rightProbe_ = RightProbeState::Blocked;
            return {
                .motion = Motion::TurnLeft,
                .reason = "right probe blocked: restore original heading",
            };
        }

        state_ = ControllerState::Cleaning;
        rightProbe_ = RightProbeState::Open;
        return {
            .motion = Motion::Forward,
            .reason = "right probe open: resume forward cleaning",
        };
    }

    if (state_ == ControllerState::EscapeAligning) {
        state_ = ControllerState::Escaping;
        rightProbe_ = RightProbeState::Blocked;
        return {
            .motion = Motion::Backward,
            .reason = "right probe blocked: original heading restored, backing up",
        };
    }

    if (state_ == ControllerState::Escaping) {
        if (!snapshot.leftObstacle) {
            state_ = ControllerState::Avoiding;
            rightProbe_ = RightProbeState::None;
            return {
                .motion = Motion::TurnLeft,
                .reason = "escaping: left opened, turn toward exit",
            };
        }

        state_ = ControllerState::RightProbing;
        rightProbe_ = RightProbeState::Checking;
        return {
            .motion = Motion::TurnRight,
            .reason = "escaping: left blocked, probe right",
        };
    }

    if (!snapshot.frontObstacle) {
        state_ = ControllerState::Cleaning;
        rightProbe_ = RightProbeState::None;
        return {
            .motion = Motion::Forward,
            .reason = "front clear: forward cleaning",
        };
    }

    if (!snapshot.leftObstacle) {
        state_ = ControllerState::Avoiding;
        rightProbe_ = RightProbeState::None;
        return {
            .motion = Motion::TurnLeft,
            .reason = "front interrupt: left open, turn left",
        };
    }

    state_ = ControllerState::RightProbing;
    rightProbe_ = RightProbeState::Checking;
    return {
        .motion = Motion::TurnRight,
        .reason = "front interrupt: left blocked, probe right",
    };
}

ControllerState NavigationPolicy::state() const {
    return state_;
}

RightProbeState NavigationPolicy::rightProbeState() const {
    return rightProbe_;
}

}  // namespace rvc
