#include "rvc/GridSimulator.hpp"

#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace rvc {

namespace {

bool startsWith(const std::string& value, const std::string& prefix) {
    return value.rfind(prefix, 0) == 0;
}

}  // namespace

GridSimulator GridSimulator::fromLines(const std::vector<std::string>& lines) {
    return GridSimulator(std::make_unique<SimulatedHardwareAdapter>(lines));
}

Scenario GridSimulator::loadScenario(const std::filesystem::path& path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("failed to open scenario file: " + path.string());
    }

    Scenario scenario;
    scenario.mapLines.clear();
    bool inMap = false;
    std::string line;

    while (std::getline(input, line)) {
        if (line.empty() || startsWith(line, "# ")) {
            continue;
        }

        if (startsWith(line, "ticks=")) {
            scenario.ticks = std::stoi(line.substr(6));
            continue;
        }

        if (line == "map:") {
            inMap = true;
            continue;
        }

        if (inMap) {
            scenario.mapLines.push_back(line);
        }
    }

    if (scenario.mapLines.empty()) {
        throw std::runtime_error("scenario file has no map section: " + path.string());
    }

    return scenario;
}

std::vector<std::string> GridSimulator::defaultMap() {
    return {
        "##########",
        "#..*.....#",
        "#.###....#",
        "#.>..*#..#",
        "#.#..#...#",
        "#........#",
        "##########",
    };
}

SimulationResult GridSimulator::run(int maxTicks, bool includeFrames) {
    if (maxTicks < 0) {
        throw std::invalid_argument("maxTicks must be non-negative");
    }

    for (int tick = 1; tick <= maxTicks; ++tick) {
        step(tick, includeFrames);
    }

    return SimulationResult{
        .ticksRun = maxTicks,
        .dustCleaned = hardwareAdapter_->dustCleaned(),
        .finalPosition = hardwareAdapter_->robotPosition(),
        .finalDirection = hardwareAdapter_->robotDirection(),
        .logs = logs_,
    };
}

bool GridSimulator::step(int tick, bool includeFrame) {
    ensureStarted();

    const Command command = rvc_.tick();
    logs_.push_back(makeLogLine(tick, rvc_.lastFrontObstacleInterrupt(), rvc_.lastPeriodicSensors(), command));
    if (includeFrame) {
        logs_.push_back(render());
    }

    return true;
}

std::string GridSimulator::render() const {
    return hardwareAdapter_->render();
}

int GridSimulator::dustCleaned() const {
    return hardwareAdapter_->dustCleaned();
}

int GridSimulator::remainingDust() const {
    return hardwareAdapter_->remainingDust();
}

Position GridSimulator::robotPosition() const {
    return hardwareAdapter_->robotPosition();
}

Direction GridSimulator::robotDirection() const {
    return hardwareAdapter_->robotDirection();
}

const std::vector<std::string>& GridSimulator::logs() const {
    return logs_;
}

GridSimulator::GridSimulator(std::unique_ptr<SimulatedHardwareAdapter> hardwareAdapter)
    : hardwareAdapter_(hardwareAdapter.get()), rvc_(std::move(hardwareAdapter)) {}

std::string GridSimulator::makeLogLine(int tick, bool frontObstacle, const PeriodicSensorData& sensors,
                                       const Command& command) const {
    const Position robot = hardwareAdapter_->robotPosition();
    const Direction direction = hardwareAdapter_->robotDirection();

    std::ostringstream output;
    output << "tick=" << tick << " frontInterrupt=" << (frontObstacle ? "true" : "false")
           << " leftPeriodic=" << (sensors.leftObstacle ? "blocked" : "open")
           << " rightPeriodic=" << (sensors.rightObstacle ? "blocked" : "open")
           << " dustPeriodic=" << (sensors.dustDetected ? "detected" : "clear")
           << " motion=" << toString(command.motion)
           << " cleaner=" << toString(command.cleaningPower)
           << " position=(" << robot.row << "," << robot.col << ")"
           << " direction=" << toString(direction)
           << " cleaned=" << hardwareAdapter_->dustCleaned()
           << " reason=\"" << command.reason << "\"";
    return output.str();
}

void GridSimulator::ensureStarted() {
    if (!started_) {
        rvc_.startCleaning();
        started_ = true;
    }
}

}  // namespace rvc
