#include "rvc/Types.hpp"

#include <gtest/gtest.h>

namespace {

using rvc::CleaningPower;
using rvc::ControllerState;
using rvc::Direction;
using rvc::Motion;

TEST(RvcTypesTest, TurnLeftCyclesThroughAllDirections) {
    EXPECT_EQ(rvc::turnLeft(Direction::North), Direction::West);
    EXPECT_EQ(rvc::turnLeft(Direction::West), Direction::South);
    EXPECT_EQ(rvc::turnLeft(Direction::South), Direction::East);
    EXPECT_EQ(rvc::turnLeft(Direction::East), Direction::North);
}

TEST(RvcTypesTest, TurnRightCyclesThroughAllDirections) {
    EXPECT_EQ(rvc::turnRight(Direction::North), Direction::East);
    EXPECT_EQ(rvc::turnRight(Direction::East), Direction::South);
    EXPECT_EQ(rvc::turnRight(Direction::South), Direction::West);
    EXPECT_EQ(rvc::turnRight(Direction::West), Direction::North);
}

TEST(RvcTypesTest, OppositeMatchesTwoRightTurns) {
    EXPECT_EQ(rvc::opposite(Direction::North), Direction::South);
    EXPECT_EQ(rvc::opposite(Direction::East), Direction::West);
    EXPECT_EQ(rvc::opposite(Direction::South), Direction::North);
    EXPECT_EQ(rvc::opposite(Direction::West), Direction::East);
}

TEST(RvcTypesTest, StringConversionsExposeStableLogNames) {
    EXPECT_EQ(rvc::toString(Direction::North), "North");
    EXPECT_EQ(rvc::toString(Direction::East), "East");
    EXPECT_EQ(rvc::toString(Direction::South), "South");
    EXPECT_EQ(rvc::toString(Direction::West), "West");

    EXPECT_EQ(rvc::toString(Motion::None), "None");
    EXPECT_EQ(rvc::toString(Motion::Stop), "Stop");
    EXPECT_EQ(rvc::toString(Motion::Forward), "Forward");
    EXPECT_EQ(rvc::toString(Motion::Backward), "Backward");
    EXPECT_EQ(rvc::toString(Motion::TurnLeft), "TurnLeft");
    EXPECT_EQ(rvc::toString(Motion::TurnRight), "TurnRight");

    EXPECT_EQ(rvc::toString(CleaningPower::Off), "Off");
    EXPECT_EQ(rvc::toString(CleaningPower::Normal), "Normal");
    EXPECT_EQ(rvc::toString(CleaningPower::Boost), "Boost");

    EXPECT_EQ(rvc::toString(ControllerState::Idle), "Idle");
    EXPECT_EQ(rvc::toString(ControllerState::Cleaning), "Cleaning");
    EXPECT_EQ(rvc::toString(ControllerState::Avoiding), "Avoiding");
    EXPECT_EQ(rvc::toString(ControllerState::Escaping), "Escaping");
}

}  // namespace
