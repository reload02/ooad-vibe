#include "rvc/CleaningPowerPolicy.hpp"

namespace rvc {

CleaningPowerPolicy::CleaningPowerPolicy(int dustBoostTicks) : dustBoostTicks_(dustBoostTicks) {}

void CleaningPowerPolicy::reset() {
    // No timer state needed in R3
}

CleaningPower CleaningPowerPolicy::update(ControllerState state, bool dustDetected) {
    // R3 규칙: 먼지를 인식해 제자리 회전할 때(DustSpinning) Boost 모드로 작동.
    // 그 외 모드에서는 일반 모드로 작동.
    if (state == ControllerState::DustSpinning) {
        return CleaningPower::Boost;
    }
    return CleaningPower::Normal;
}

int CleaningPowerPolicy::boostTicksRemaining() const {
    return 0;
}

}  // namespace rvc
