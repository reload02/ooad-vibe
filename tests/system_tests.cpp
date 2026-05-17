#include "rvc/GridSimulator.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <vector>

namespace {

using rvc::Direction;
using rvc::GridSimulator;
using rvc::Position;

bool containsLog(const std::vector<std::string>& logs, const std::string& text) {
    return std::any_of(logs.begin(), logs.end(), [&](const std::string& line) {
        return line.find(text) != std::string::npos;
    });
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
    EXPECT_EQ(result.finalPosition, (Position{4, 2}));
    EXPECT_EQ(result.finalDirection, Direction::East);
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

}  // namespace
