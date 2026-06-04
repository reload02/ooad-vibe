#pragma once

#include "rvc/Rvc.hpp"
#include "rvc/SimulatedHardwareAdapter.hpp"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace rvc {

struct Scenario {
    std::vector<std::string> mapLines;
    int ticks{20};
};

struct SimulationResult {
    int ticksRun{0};
    int dustCleaned{0};
    Position finalPosition{};
    Direction finalDirection{Direction::North};
    std::vector<std::string> logs;
};

class GridSimulator {
public:
    static GridSimulator fromLines(const std::vector<std::string>& lines);
    static Scenario loadScenario(const std::filesystem::path& path);
    static std::vector<std::string> defaultMap();

    SimulationResult run(int maxTicks, bool includeFrames);
    bool step(int tick, bool includeFrame);

    [[nodiscard]] std::string render() const;
    [[nodiscard]] int dustCleaned() const;
    [[nodiscard]] int remainingDust() const;
    [[nodiscard]] Position robotPosition() const;
    [[nodiscard]] Direction robotDirection() const;
    [[nodiscard]] const std::vector<std::string>& logs() const;

private:
    explicit GridSimulator(std::unique_ptr<SimulatedHardwareAdapter> hardwareAdapter);

    [[nodiscard]] std::string makeLogLine(int tick, bool frontObstacle, const PeriodicSensorData& sensors,
                                          const Command& command) const;

    void ensureStarted();

    SimulatedHardwareAdapter* hardwareAdapter_;
    Rvc rvc_;
    bool started_{false};
    std::vector<std::string> logs_;
};

}  // namespace rvc
