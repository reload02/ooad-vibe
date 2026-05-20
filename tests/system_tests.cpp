#include "rvc/GridSimulator.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

#ifndef RVC_SOURCE_DIR
#define RVC_SOURCE_DIR "."
#endif

namespace {

using rvc::Direction;
using rvc::GridSimulator;
using rvc::Position;

bool containsLog(const std::vector<std::string>& logs, const std::string& text) {
    return std::any_of(logs.begin(), logs.end(), [&](const std::string& line) {
        return line.find(text) != std::string::npos;
    });
}

int countLogs(const std::vector<std::string>& logs, const std::string& text) {
    return static_cast<int>(std::count_if(logs.begin(), logs.end(), [&](const std::string& line) {
        return line.find(text) != std::string::npos;
    }));
}

std::filesystem::path scenarioPath(const std::string& file) {
    return std::filesystem::path(RVC_SOURCE_DIR) / "scenarios" / file;
}

std::filesystem::path errorScenarioPath(const std::string& file) {
    return std::filesystem::path(RVC_SOURCE_DIR) / "scenarios" / "error_cases" / file;
}

struct ScenarioExpectation {
    const char* file;
    int ticks;
    Position finalPosition;
    Direction finalDirection;
    int dustCleaned;
    int remainingDust;
    int forwardCommands;
    int leftTurns;
    int rightTurns;
    int backwardCommands;
    int boostCommands;
    int offCommands;
    std::vector<std::string> requiredLogFragments;
};

void PrintTo(const ScenarioExpectation& expected, std::ostream* output) {
    *output << expected.file;
}

std::string scenarioName(const ::testing::TestParamInfo<ScenarioExpectation>& info) {
    std::string name = info.param.file;
    for (char& character : name) {
        const bool isDigit = character >= '0' && character <= '9';
        const bool isLower = character >= 'a' && character <= 'z';
        const bool isUpper = character >= 'A' && character <= 'Z';
        if (!isDigit && !isLower && !isUpper) {
            character = '_';
        }
    }
    return name;
}

TEST(RvcSystemTest, SimulatorCleansDustAndLogsCommands) {
    GridSimulator simulator = GridSimulator::fromLines({
        "#####",
        "#^*.#",
        "#...#",
        "#####",
    });

    const rvc::SimulationResult result = simulator.run(4, false);

    EXPECT_GE(result.dustCleaned, 1);
    EXPECT_TRUE(containsLog(result.logs, "dustPeriodic=detected"));
    EXPECT_TRUE(containsLog(result.logs, "cleaner=Boost"));
}

TEST(RvcSystemTest, SimulatorUsesBackwardEscape) {
    GridSimulator simulator = GridSimulator::fromLines({
        "#####",
        "#####",
        "##^##",
        "#...#",
        "#####",
    });

    const rvc::SimulationResult result = simulator.run(1, false);

    EXPECT_TRUE(containsLog(result.logs, "motion=Backward"));
    EXPECT_TRUE(containsLog(result.logs, "motion=Backward cleaner=Off"));
    EXPECT_EQ(result.finalPosition, (Position{3, 2}));
    EXPECT_EQ(result.finalDirection, Direction::North);
}

TEST(RvcSystemTest, SimulatorKeepsCommandingBackwardWhenBoxedIn) {
    GridSimulator simulator = GridSimulator::fromLines({
        "#####",
        "#####",
        "##^##",
        "#####",
        "#####",
    });

    const rvc::SimulationResult result = simulator.run(5, false);
    const int backwardCommands = static_cast<int>(std::count_if(result.logs.begin(), result.logs.end(), [](const auto& line) {
        return line.find("motion=Backward") != std::string::npos;
    }));

    EXPECT_EQ(backwardCommands, 5);
    EXPECT_EQ(result.finalPosition, (Position{2, 2}));
    EXPECT_EQ(result.finalDirection, Direction::North);
}

TEST(RvcSystemTest, SimulatorKeepsBackingUpUntilSideExitOpens) {
    GridSimulator simulator = GridSimulator::fromLines({
        "#####",
        "#####",
        "##^##",
        "##.##",
        "##..#",
        "#####",
    });

    const rvc::SimulationResult result = simulator.run(3, false);
    const int backwardCommands = static_cast<int>(std::count_if(result.logs.begin(), result.logs.end(), [](const auto& line) {
        return line.find("motion=Backward") != std::string::npos;
    }));

    EXPECT_EQ(backwardCommands, 2);
    EXPECT_TRUE(containsLog(result.logs, "tick=2 frontInterrupt=false"));
    EXPECT_TRUE(containsLog(result.logs, "tick=2 frontInterrupt=false leftPeriodic=blocked rightPeriodic=blocked"));
    EXPECT_TRUE(containsLog(result.logs, "tick=3 frontInterrupt=false leftPeriodic=blocked rightPeriodic=open"));
    EXPECT_TRUE(containsLog(result.logs, "motion=TurnRight"));
    EXPECT_TRUE(containsLog(result.logs, "motion=TurnRight cleaner=Off"));
    EXPECT_EQ(result.finalPosition, (Position{4, 2}));
    EXPECT_EQ(result.finalDirection, Direction::East);
}

TEST(RvcSystemTest, SimulatorKeepsCleanerOffDuringBoostedEscape) {
    GridSimulator simulator = GridSimulator::fromLines({
        "#######",
        "#>*..##",
        "###.###",
        "###.###",
        "#######",
    });

    const rvc::SimulationResult result = simulator.run(5, false);

    EXPECT_TRUE(containsLog(result.logs, "motion=Forward cleaner=Boost"));
    EXPECT_TRUE(containsLog(result.logs, "motion=Backward cleaner=Off"));
    EXPECT_TRUE(containsLog(result.logs, "motion=TurnRight cleaner=Off"));
}

TEST(RvcSystemTest, SimulatorTurnsAfterFrontInterrupt) {
    GridSimulator simulator = GridSimulator::fromLines({
        "#####",
        "#.#.#",
        "##^.#",
        "#...#",
        "#####",
    });

    const rvc::SimulationResult result = simulator.run(1, false);

    EXPECT_TRUE(containsLog(result.logs, "frontInterrupt=true"));
    EXPECT_TRUE(containsLog(result.logs, "motion=TurnRight"));
    EXPECT_EQ(result.finalDirection, Direction::East);
}

TEST(RvcSystemTest, SimulatorRunWithZeroTicksLeavesRobotAtInitialPose) {
    GridSimulator simulator = GridSimulator::fromLines({
        "#####",
        "#>*.#",
        "#####",
    });

    const rvc::SimulationResult result = simulator.run(0, false);

    EXPECT_EQ(result.ticksRun, 0);
    EXPECT_TRUE(result.logs.empty());
    EXPECT_EQ(result.dustCleaned, 0);
    EXPECT_EQ(simulator.remainingDust(), 1);
    EXPECT_EQ(result.finalPosition, (Position{1, 1}));
    EXPECT_EQ(result.finalDirection, Direction::East);
}

TEST(RvcSystemTest, SimulatorRejectsNegativeTicks) {
    GridSimulator simulator = GridSimulator::fromLines({
        "#####",
        "#R..#",
        "#####",
    });

    EXPECT_THROW((void)simulator.run(-1, false), std::invalid_argument);
}

TEST(RvcSystemTest, SimulatorCanAppendRenderedFrames) {
    GridSimulator simulator = GridSimulator::fromLines({
        "#####",
        "#>..#",
        "#####",
    });

    const rvc::SimulationResult result = simulator.run(1, true);

    ASSERT_EQ(result.logs.size(), 2U);
    EXPECT_TRUE(containsLog(result.logs, "motion=Forward"));
    EXPECT_NE(result.logs[1].find("#.>.#"), std::string::npos);
}

TEST(RvcScenarioFileTest, LoadsScenarioFileWithTicksAndMap) {
    const rvc::Scenario scenario = GridSimulator::loadScenario(scenarioPath("dust_and_interrupt.rvc"));

    EXPECT_EQ(scenario.ticks, 10);
    ASSERT_EQ(scenario.mapLines.size(), 7U);
    EXPECT_EQ(scenario.mapLines[3], "#.>..*#..#");
}

TEST(RvcScenarioFileTest, RejectsMissingOrEmptyMapFilesDuringLoad) {
    EXPECT_THROW((void)GridSimulator::loadScenario(errorScenarioPath("missing_map_invalid.rvc")), std::runtime_error);
    EXPECT_THROW((void)GridSimulator::loadScenario(errorScenarioPath("empty_map_invalid.rvc")), std::runtime_error);
}

TEST(RvcScenarioFileTest, RejectsRobotMarkerErrorsAfterLoadingMap) {
    const rvc::Scenario noRobot = GridSimulator::loadScenario(errorScenarioPath("no_robot_invalid.rvc"));
    const rvc::Scenario multipleRobots = GridSimulator::loadScenario(errorScenarioPath("multiple_robots_invalid.rvc"));

    EXPECT_THROW((void)GridSimulator::fromLines(noRobot.mapLines), std::invalid_argument);
    EXPECT_THROW((void)GridSimulator::fromLines(multipleRobots.mapLines), std::invalid_argument);
}

TEST(RvcScenarioFileTest, RejectsNegativeTickScenarioWhenRun) {
    const rvc::Scenario scenario = GridSimulator::loadScenario(errorScenarioPath("negative_ticks_invalid.rvc"));
    GridSimulator simulator = GridSimulator::fromLines(scenario.mapLines);

    EXPECT_EQ(scenario.ticks, -1);
    EXPECT_THROW((void)simulator.run(scenario.ticks, false), std::invalid_argument);
}

TEST(RvcScenarioFileTest, UnknownSymbolScenarioDocumentsCurrentValidationGap) {
    const rvc::Scenario scenario = GridSimulator::loadScenario(errorScenarioPath("unknown_symbol_validation_gap.rvc"));
    GridSimulator simulator = GridSimulator::fromLines(scenario.mapLines);

    const rvc::SimulationResult result = simulator.run(scenario.ticks, false);

    EXPECT_EQ(result.ticksRun, 2);
    EXPECT_EQ(result.finalPosition, (Position{0, 1}));
    EXPECT_EQ(result.finalDirection, Direction::West);
    EXPECT_TRUE(containsLog(result.logs, "motion=Forward cleaner=Normal"));
    EXPECT_TRUE(containsLog(result.logs, "frontInterrupt=true"));
}

class RvcScenarioRegressionTest : public ::testing::TestWithParam<ScenarioExpectation> {};

TEST_P(RvcScenarioRegressionTest, ScenarioFileRunsWithExpectedOutcome) {
    const ScenarioExpectation& expected = GetParam();
    const rvc::Scenario scenario = GridSimulator::loadScenario(scenarioPath(expected.file));
    GridSimulator simulator = GridSimulator::fromLines(scenario.mapLines);

    const rvc::SimulationResult result = simulator.run(scenario.ticks, false);

    EXPECT_EQ(scenario.ticks, expected.ticks);
    EXPECT_EQ(result.ticksRun, expected.ticks);
    EXPECT_EQ(result.logs.size(), static_cast<std::size_t>(expected.ticks));
    EXPECT_EQ(result.finalPosition, expected.finalPosition);
    EXPECT_EQ(result.finalDirection, expected.finalDirection);
    EXPECT_EQ(result.dustCleaned, expected.dustCleaned);
    EXPECT_EQ(simulator.remainingDust(), expected.remainingDust);
    EXPECT_EQ(countLogs(result.logs, "motion=Forward"), expected.forwardCommands);
    EXPECT_EQ(countLogs(result.logs, "motion=TurnLeft"), expected.leftTurns);
    EXPECT_EQ(countLogs(result.logs, "motion=TurnRight"), expected.rightTurns);
    EXPECT_EQ(countLogs(result.logs, "motion=Backward"), expected.backwardCommands);
    EXPECT_EQ(countLogs(result.logs, "cleaner=Boost"), expected.boostCommands);
    EXPECT_EQ(countLogs(result.logs, "cleaner=Off"), expected.offCommands);

    for (const std::string& fragment : expected.requiredLogFragments) {
        EXPECT_TRUE(containsLog(result.logs, fragment)) << expected.file << " missing: " << fragment;
    }
}

INSTANTIATE_TEST_SUITE_P(
    ScenarioFiles,
    RvcScenarioRegressionTest,
    ::testing::Values(
        ScenarioExpectation{"backward_escape.rvc", 4, Position{3, 2}, Direction::West, 0, 0, 1, 1, 0, 2, 0, 3,
                            {"escaping: side opened, turn toward exit"}},
        ScenarioExpectation{"backward_escape2.rvc", 20, Position{6, 2}, Direction::North, 0, 0, 7, 1, 1, 11, 0, 13,
                            {"motion=TurnRight cleaner=Off"}},
        ScenarioExpectation{"boundary_without_outer_wall.rvc", 4, Position{2, 1}, Direction::East, 0, 0, 2, 2, 0, 0,
                            0, 2, {"frontInterrupt=true"}},
        ScenarioExpectation{"clear_corridor_forward.rvc", 8, Position{1, 9}, Direction::East, 0, 0, 8, 0, 0, 0, 0, 0,
                            {"front clear: forward cleaning"}},
        ScenarioExpectation{"continuous_backward.rvc", 5, Position{2, 2}, Direction::North, 0, 0, 0, 0, 0, 5, 0, 5,
                            {"escaping: keep backing up until a side exit opens"}},
        ScenarioExpectation{"dense_dust_maze_extreme.rvc", 50, Position{3, 5}, Direction::East, 7, 0, 38, 10, 2, 0,
                            11, 12, {"motion=Forward cleaner=Boost", "motion=TurnLeft cleaner=Off"}},
        ScenarioExpectation{"dust_and_interrupt.rvc", 10, Position{1, 2}, Direction::West, 2, 0, 8, 2, 0, 0, 3, 2,
                            {"dustPeriodic=detected"}},
        ScenarioExpectation{"dust_before_dead_end_escape.rvc", 8, Position{2, 3}, Direction::South, 1, 0, 5, 0, 1, 2,
                            2, 3, {"motion=Backward cleaner=Off"}},
        ScenarioExpectation{"dust_trail_boost_refresh.rvc", 8, Position{1, 8}, Direction::South, 3, 0, 7, 0, 1, 0,
                            6, 1, {"motion=Forward cleaner=Boost"}},
        ScenarioExpectation{"front_both_sides_open.rvc", 3, Position{2, 1}, Direction::North, 0, 0, 1, 1, 1, 0, 0, 2,
                            {"motion=TurnLeft cleaner=Off", "motion=TurnRight cleaner=Off"}},
        ScenarioExpectation{"front_clears_but_sides_still_blocked.rvc", 6, Position{2, 2}, Direction::South, 0, 0, 3,
                            2, 0, 1, 0, 3, {"motion=Backward cleaner=Off"}},
        ScenarioExpectation{"front_left_only_open.rvc", 2, Position{2, 1}, Direction::West, 0, 0, 1, 1, 0, 0, 0, 1,
                            {"motion=TurnLeft cleaner=Off"}},
        ScenarioExpectation{"front_right_only_open.rvc", 2, Position{2, 3}, Direction::East, 0, 0, 1, 0, 1, 0, 0, 1,
                            {"motion=TurnRight cleaner=Off"}},
        ScenarioExpectation{"large_open_room_dust_sweep.rvc", 30, Position{5, 1}, Direction::West, 3, 2, 28, 0, 2, 0,
                            9, 2, {"dustPeriodic=detected"}},
        ScenarioExpectation{"long_escape_left_exit_extreme.rvc", 9, Position{9, 1}, Direction::West, 0, 0, 1, 1, 0, 7,
                            0, 8, {"motion=TurnLeft cleaner=Off"}},
        ScenarioExpectation{"long_escape_right_exit_extreme.rvc", 9, Position{9, 3}, Direction::East, 0, 0, 1, 0, 1,
                            7, 0, 8, {"motion=TurnRight cleaner=Off"}},
        ScenarioExpectation{"narrow_tunnel_sides_blocked_front_clear.rvc", 6, Position{1, 5}, Direction::East, 0, 0, 5,
                            0, 0, 1, 0, 1, {"leftPeriodic=blocked rightPeriodic=blocked dustPeriodic=clear motion=Forward"}},
        ScenarioExpectation{"ragged_map_edge_extreme.rvc", 8, Position{0, 0}, Direction::West, 1, 0, 6, 2, 0, 0, 2, 2,
                            {"frontInterrupt=true"}},
        ScenarioExpectation{"repeated_front_interrupts_alternation.rvc", 12, Position{2, 7}, Direction::East, 0, 0, 8,
                            1, 2, 1, 0, 4, {"motion=TurnLeft cleaner=Off", "motion=TurnRight cleaner=Off"}},
        ScenarioExpectation{"sealed_box_extreme.rvc", 10, Position{2, 2}, Direction::North, 0, 0, 0, 0, 0, 10, 0, 10,
                            {"position=(2,2) direction=North"}}),
    scenarioName);

}  // namespace
