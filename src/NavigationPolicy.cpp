#include "rvc/NavigationPolicy.hpp"

namespace rvc {

void NavigationPolicy::startCleaning() {
    state_ = ControllerState::Cleaning;
    rightProbe_ = RightProbeState::None;
    wasForward_ = true;
    savedState_ = ControllerState::Idle;
    spinTicks_ = 0;
    isLeavingDustCell_ = false;
}

void NavigationPolicy::stopCleaning() {
    state_ = ControllerState::Idle;
    rightProbe_ = RightProbeState::None;
    wasForward_ = true;
    savedState_ = ControllerState::Idle;
    spinTicks_ = 0;
    isLeavingDustCell_ = false;
}

NavigationDecision NavigationPolicy::decideInternal(const SensorSnapshot& snapshot) {
    if (state_ == ControllerState::Idle) {
        return {
            .motion = Motion::Stop,
            .reason = "navigation idle",
        };
    }

    // 0. 예약된 상태 전이 처리 (Tick 시작 시점)
    if (state_ == ControllerState::DustSpinning && spinTicks_ <= 0) {
        state_ = ControllerState::DustLeaving;
    }

    // 1. 먼지 감지 및 이탈 상태 제어 (FSM 우선순위 1)
    if (state_ == ControllerState::DustSpinning) {
        --spinTicks_;
        std::string directionText = wasForward_ ? "clockwise" : "counter-clockwise of progress direction";
        return {
            .motion = Motion::TurnRight,
            .reason = "dust spin boost: rotating 180 degrees (" + directionText + ")",
        };
    }

    if (state_ == ControllerState::DustLeaving) {
        if (snapshot.frontObstacle) {
            if (!snapshot.leftObstacle) {
                state_ = ControllerState::Avoiding;
                rightProbe_ = RightProbeState::None;
                return {
                    .motion = Motion::TurnLeft,
                    .reason = "dust leaving check: front blocked, left open, turn left to avoid",
                };
            } else {
                state_ = ControllerState::RightProbing;
                rightProbe_ = RightProbeState::Checking;
                return {
                    .motion = Motion::TurnRight,
                    .reason = "dust leaving check: front blocked, left blocked, probe right to avoid",
                };
            }
        }

        if (wasForward_) {
            if (snapshot.backwardObstacle) {
                state_ = ControllerState::Escaping;
                rightProbe_ = RightProbeState::Blocked;
                return {
                    .motion = Motion::Stop,
                    .reason = "dust leaving check: backward blocked, stop and escape",
                };
            }
            state_ = ControllerState::DustLeavingBackward;
            return {
                .motion = Motion::Stop,
                .reason = "dust leaving check: backward clear, check sensors for 1 tick",
            };
        } else {
            state_ = ControllerState::DustLeavingForward;
            return {
                .motion = Motion::Stop,
                .reason = "dust leaving check: front clear, check sensors for 1 tick",
            };
        }
    }

    if (state_ == ControllerState::DustLeavingBackward) {
        state_ = ControllerState::Escaping;
        rightProbe_ = RightProbeState::Blocked;
        if (snapshot.backwardObstacle) {
            return {
                .motion = Motion::Stop,
                .reason = "dust leaving backward: backward blocked, stop motion",
            };
        }
        return {
            .motion = Motion::Backward,
            .reason = "dust leaving backward: escaping dust cell",
        };
    }

    if (state_ == ControllerState::DustLeavingForward) {
        state_ = ControllerState::Cleaning;
        rightProbe_ = RightProbeState::None;
        return {
            .motion = Motion::Forward,
            .reason = "dust leaving forward: escaping dust cell",
        };
    }

    if (snapshot.dustDetected && !isLeavingDustCell_) {
        isLeavingDustCell_ = true;
        savedState_ = state_;
        state_ = ControllerState::DustSpinning;
        spinTicks_ = 2;
        --spinTicks_;
        std::string directionText = wasForward_ ? "clockwise" : "counter-clockwise of progress direction";
        return {
            .motion = Motion::TurnRight,
            .reason = "dust spin boost: rotating 180 degrees (" + directionText + ")",
        };
    }

    // 2. 기존 장애물 판단 FSM
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
        wasForward_ = true;
        return {
            .motion = Motion::Forward,
            .reason = "right probe open: resume forward cleaning",
        };
    }

    if (state_ == ControllerState::EscapeAligning) {
        state_ = ControllerState::Escaping;
        rightProbe_ = RightProbeState::Blocked;
        wasForward_ = false;
        if (snapshot.backwardObstacle) {
            return {
                .motion = Motion::Stop,
                .reason = "right probe blocked: original heading restored, backing up (backward blocked, stop)",
            };
        }
        return {
            .motion = Motion::Backward,
            .reason = "right probe blocked: original heading restored, backing up",
        };
    }

    if (state_ == ControllerState::Escaping) {
        if (snapshot.backwardObstacle) {
            return {
                .motion = Motion::Stop,
                .reason = "escaping: backward blocked, stop motion",
            };
        }

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
        wasForward_ = true;
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

NavigationDecision NavigationPolicy::decide(const SensorSnapshot& snapshot) {
    NavigationDecision decision = decideInternal(snapshot);
    if (decision.motion == Motion::Forward || decision.motion == Motion::Backward) {
        isLeavingDustCell_ = false;
    }
    return decision;
}

}  // namespace rvc
