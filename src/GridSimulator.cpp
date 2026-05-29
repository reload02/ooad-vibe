#include "rvc/GridSimulator.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace rvc {

namespace {

char directionMarker(Direction direction) {
    switch (direction) {
        case Direction::North:
            return '^';
        case Direction::East:
            return '>';
        case Direction::South:
            return 'v';
        case Direction::West:
            return '<';
    }
    return '?';
}

Direction directionFromMarker(char marker) {
    switch (marker) {
        case '^':
            return Direction::North;
        case '>':
            return Direction::East;
        case 'v':
            return Direction::South;
        case '<':
            return Direction::West;
        default:
            return Direction::North;
    }
}

bool startsWith(const std::string& value, const std::string& prefix) {
    return value.rfind(prefix, 0) == 0;
}

}  // namespace

GridSimulator GridSimulator::fromLines(const std::vector<std::string>& lines) {
    if (lines.empty()) {
        throw std::invalid_argument("scenario map must not be empty");
    }

    GridSimulator simulator(lines);
    bool robotFound = false;

    for (int row = 0; row < static_cast<int>(simulator.grid_.size()); ++row) {
        for (int col = 0; col < static_cast<int>(simulator.grid_[row].size()); ++col) {
            const char cell = simulator.grid_[row][col];
            if (cell == 'R' || cell == '^' || cell == '>' || cell == 'v' || cell == '<') {
                if (robotFound) {
                    throw std::invalid_argument("scenario map must contain only one robot");
                }
                robotFound = true;
                simulator.robot_ = Position{row, col};
                simulator.direction_ = cell == 'R' ? Direction::North : directionFromMarker(cell);
                simulator.grid_[row][col] = '.';
            }
        }
    }

    if (!robotFound) {
        throw std::invalid_argument("scenario map must contain a robot marker");
    }

    return simulator;
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
        .dustCleaned = dustCleaned_,
        .finalPosition = robot_,
        .finalDirection = direction_,
        .logs = logs_,
    };
}

bool GridSimulator::step(int tick, bool includeFrame) {
    ensureStarted();

    const bool frontObstacle = isObstacle(forwardPosition());
    if (frontObstacle) {
        rvc_.onFrontObstacleInterrupt();
    }

    const PeriodicSensorData sensors = samplePeriodicSensors();
    const Command command = rvc_.tick(sensors);

    cleanCurrentCell(command);
    applyMotion(command.motion);

    logs_.push_back(makeLogLine(tick, frontObstacle, sensors, command));
    if (includeFrame) {
        logs_.push_back(render());
    }

    return true;
}

std::string GridSimulator::render() const {
    std::vector<std::string> copy = grid_;
    if (robot_.row >= 0 && robot_.row < static_cast<int>(copy.size()) && robot_.col >= 0 &&
        robot_.col < static_cast<int>(copy[robot_.row].size())) {
        copy[robot_.row][robot_.col] = directionMarker(direction_);
    }

    std::ostringstream output;
    for (const auto& line : copy) {
        output << line << '\n';
    }
    return output.str();
}

int GridSimulator::dustCleaned() const {
    return dustCleaned_;
}

int GridSimulator::remainingDust() const {
    int count = 0;
    for (const auto& row : grid_) {
        count += static_cast<int>(std::count(row.begin(), row.end(), '*'));
    }
    return count;
}

Position GridSimulator::robotPosition() const {
    return robot_;
}

Direction GridSimulator::robotDirection() const {
    return direction_;
}

const std::vector<std::string>& GridSimulator::logs() const {
    return logs_;
}

GridSimulator::GridSimulator(std::vector<std::string> grid) : grid_(std::move(grid)) {}

PeriodicSensorData GridSimulator::samplePeriodicSensors() const {
    return PeriodicSensorData{
        .leftObstacle = isObstacle(adjacent(robot_, turnLeft(direction_))),
        .rightObstacle = isObstacle(adjacent(robot_, turnRight(direction_))),
        .dustDetected = hasDust(robot_),
    };
}

bool GridSimulator::isObstacle(Position position) const {
    if (position.row < 0 || position.row >= static_cast<int>(grid_.size())) {
        return true;
    }

    if (position.col < 0 || position.col >= static_cast<int>(grid_[position.row].size())) {
        return true;
    }

    return grid_[position.row][position.col] == '#';
}

bool GridSimulator::hasDust(Position position) const {
    if (position.row < 0 || position.row >= static_cast<int>(grid_.size())) {
        return false;
    }

    if (position.col < 0 || position.col >= static_cast<int>(grid_[position.row].size())) {
        return false;
    }

    return grid_[position.row][position.col] == '*';
}

Position GridSimulator::adjacent(Position origin, Direction direction) const {
    switch (direction) {
        case Direction::North:
            return Position{origin.row - 1, origin.col};
        case Direction::East:
            return Position{origin.row, origin.col + 1};
        case Direction::South:
            return Position{origin.row + 1, origin.col};
        case Direction::West:
            return Position{origin.row, origin.col - 1};
    }
    return origin;
}

Position GridSimulator::forwardPosition() const {
    return adjacent(robot_, direction_);
}

Position GridSimulator::backwardPosition() const {
    return adjacent(robot_, opposite(direction_));
}

std::string GridSimulator::makeLogLine(int tick, bool frontObstacle, const PeriodicSensorData& sensors,
                                       const Command& command) const {
    std::ostringstream output;
    output << "tick=" << tick << " frontInterrupt=" << (frontObstacle ? "true" : "false")
           << " leftPeriodic=" << (sensors.leftObstacle ? "blocked" : "open")
           << " rightPeriodic=" << (sensors.rightObstacle ? "blocked" : "open")
           << " dustPeriodic=" << (sensors.dustDetected ? "detected" : "clear")
           << " motion=" << toString(command.motion)
           << " cleaner=" << toString(command.cleaningPower)
           << " position=(" << robot_.row << "," << robot_.col << ")"
           << " direction=" << toString(direction_)
           << " cleaned=" << dustCleaned_
           << " reason=\"" << command.reason << "\"";
    return output.str();
}

void GridSimulator::cleanCurrentCell(const Command& command) {
    if (command.cleaningPower == CleaningPower::Off || !hasDust(robot_)) {
        return;
    }

    grid_[robot_.row][robot_.col] = '.';
    ++dustCleaned_;
}

void GridSimulator::applyMotion(Motion motion) {
    switch (motion) {
        case Motion::Forward: {
            const Position target = forwardPosition();
            if (!isObstacle(target)) {
                robot_ = target;
            }
            break;
        }
        case Motion::Backward: {
            const Position target = backwardPosition();
            if (!isObstacle(target)) {
                robot_ = target;
            }
            break;
        }
        case Motion::TurnLeft:
            direction_ = turnLeft(direction_);
            break;
        case Motion::TurnRight:
            direction_ = turnRight(direction_);
            break;
        case Motion::None:
        case Motion::Stop:
            break;
    }
}

void GridSimulator::ensureStarted() {
    if (!started_) {
        rvc_.startCleaning();
        started_ = true;
    }
}

}  // namespace rvc
