#include "rvc/CleaningPowerPolicy.hpp"

namespace rvc {

CleaningPowerPolicy::CleaningPowerPolicy(int dustBoostTicks) : dustBoostTicks_(dustBoostTicks) {}

void CleaningPowerPolicy::reset() {
    boostTicksRemaining_ = 0;
}

CleaningPower CleaningPowerPolicy::update(bool dustDetected) {
    if (dustDetected) {
        boostTicksRemaining_ = dustBoostTicks_;
    } else if (boostTicksRemaining_ > 0) {
        --boostTicksRemaining_;
    }

    return boostTicksRemaining_ > 0 ? CleaningPower::Boost : CleaningPower::Normal;
}

int CleaningPowerPolicy::boostTicksRemaining() const {
    return boostTicksRemaining_;
}

}  // namespace rvc
