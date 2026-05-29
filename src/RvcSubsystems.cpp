#include "rvc/RvcSubsystems.hpp"

#include <utility>

namespace rvc {

void SensorFusion::recordFrontObstacleInterrupt() {
    frontInterruptPending_ = true;
}

void SensorFusion::clearFrontObstacleInterrupt() {
    frontInterruptPending_ = false;
}

SensorSnapshot SensorFusion::fuse(const PeriodicSensorData& periodicSensors) const {
    return SensorSnapshot{
        .frontObstacle = frontInterruptPending_,
        .leftObstacle = periodicSensors.leftObstacle,
        .dustDetected = periodicSensors.dustDetected,
    };
}

bool SensorFusion::frontInterruptPending() const {
    return frontInterruptPending_;
}

void NavigationPolicy::startCleaning() {
    state_ = ControllerState::Cleaning;
    escapeProbePhase_ = EscapeProbePhase::BackingUp;
}

void NavigationPolicy::stopCleaning() {
    state_ = ControllerState::Idle;
    escapeProbePhase_ = EscapeProbePhase::BackingUp;
}

NavigationDecision NavigationPolicy::decide(const SensorSnapshot& snapshot) {
    if (state_ == ControllerState::Escaping) {
        if (escapeProbePhase_ == EscapeProbePhase::EvaluatingRightProbe) {
            if (!snapshot.frontObstacle) {
                state_ = ControllerState::Cleaning;
                escapeProbePhase_ = EscapeProbePhase::BackingUp;
                return NavigationDecision{
                    .motion = Motion::Forward,
                    .reason = "escaping: right probe clear, move into exit",
                };
            }

            escapeProbePhase_ = EscapeProbePhase::BackingUp;
            return NavigationDecision{
                .motion = Motion::TurnLeft,
                .reason = "escaping: right probe blocked, restore heading",
            };
        }

        if (!snapshot.leftObstacle) {
            state_ = ControllerState::Avoiding;
            escapeProbePhase_ = EscapeProbePhase::BackingUp;
            return NavigationDecision{
                .motion = Motion::TurnLeft,
                .reason = "escaping: left side opened, turn toward exit",
            };
        }

        if (escapeProbePhase_ == EscapeProbePhase::TurningRight) {
            escapeProbePhase_ = EscapeProbePhase::EvaluatingRightProbe;
            return NavigationDecision{
                .motion = Motion::TurnRight,
                .reason = "escaping: turn right to probe exit with front interrupt",
            };
        }

        escapeProbePhase_ = EscapeProbePhase::TurningRight;
        return NavigationDecision{
            .motion = Motion::Backward,
            .reason = "escaping: back up before probing right with front interrupt",
        };
    }

    if (!snapshot.frontObstacle) {
        state_ = ControllerState::Cleaning;
        escapeProbePhase_ = EscapeProbePhase::BackingUp;
        return NavigationDecision{
            .motion = Motion::Forward,
            .reason = "front clear: forward cleaning",
        };
    }

    if (!snapshot.leftObstacle) {
        state_ = ControllerState::Avoiding;
        escapeProbePhase_ = EscapeProbePhase::BackingUp;
        return NavigationDecision{
            .motion = Motion::TurnLeft,
            .reason = "front interrupt: left side open, turn left",
        };
    }

    state_ = ControllerState::Escaping;
    escapeProbePhase_ = EscapeProbePhase::TurningRight;
    return NavigationDecision{
        .motion = Motion::Backward,
        .reason = "front interrupt and left side blocked: enter escaping",
    };
}

ControllerState NavigationPolicy::state() const {
    return state_;
}

CleaningPolicy::CleaningPolicy(ControllerConfig config) : config_(config) {}

void CleaningPolicy::reset() {
    boostTicksRemaining_ = 0;
}

CleaningPower CleaningPolicy::update(bool dustDetected) {
    if (dustDetected) {
        boostTicksRemaining_ = config_.dustBoostTicks;
    } else if (boostTicksRemaining_ > 0) {
        --boostTicksRemaining_;
    }

    return boostTicksRemaining_ > 0 ? CleaningPower::Boost : CleaningPower::Normal;
}

int CleaningPolicy::boostTicksRemaining() const {
    return boostTicksRemaining_;
}

Command CommandComposer::compose(Motion motion, CleaningPower power, std::string reason) const {
    const CleaningPower outputPower = motion == Motion::Forward ? power : CleaningPower::Off;

    return Command{
        .motion = motion,
        .cleaningPower = outputPower,
        .reason = std::move(reason),
    };
}

}  // namespace rvc
