#pragma once

#include "rvc/Types.hpp"

#include <string>

namespace rvc {

struct NavigationDecision {
    Motion motion{Motion::None};
    std::string reason;
};

class NavigationPolicy {
public:
    void startCleaning();
    void stopCleaning();

    [[nodiscard]] NavigationDecision decide(const SensorSnapshot& snapshot);
    [[nodiscard]] ControllerState state() const;
    [[nodiscard]] RightProbeState rightProbeState() const;

private:
    ControllerState state_{ControllerState::Idle};
    RightProbeState rightProbe_{RightProbeState::None};
};

}  // namespace rvc
