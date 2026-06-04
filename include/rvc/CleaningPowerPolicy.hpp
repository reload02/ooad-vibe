#pragma once

#include "rvc/Types.hpp"

namespace rvc {

class CleaningPowerPolicy {
public:
    explicit CleaningPowerPolicy(int dustBoostTicks);

    void reset();

    [[nodiscard]] CleaningPower update(bool dustDetected);
    [[nodiscard]] int boostTicksRemaining() const;

private:
    int dustBoostTicks_{0};
    int boostTicksRemaining_{0};
};

}  // namespace rvc
