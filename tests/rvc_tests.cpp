#include "rvc/Rvc.hpp"

#include <gtest/gtest.h>

#include <memory>

namespace {

class FakeHardwareAdapter final : public rvc::RvcHardwareAdapter {
public:
    bool frontObstacle{false};
    bool backwardObstacle{false};
    rvc::PeriodicSensorData sensors{};
    rvc::Command appliedCommand{};
    int applyCount{0};

    [[nodiscard]] bool hasFrontObstacleInterrupt() const override {
        return frontObstacle;
    }

    [[nodiscard]] bool hasBackwardObstacleInterrupt() const override {
        return backwardObstacle;
    }

    [[nodiscard]] rvc::PeriodicSensorData readPeriodicSensors() const override {
        return sensors;
    }

    void applyCommand(const rvc::Command& command) override {
        appliedCommand = command;
        ++applyCount;
    }
};

TEST(RvcTest, TickReadsAdapterInputsAndAppliesControllerCommand) {
    auto adapter = std::make_unique<FakeHardwareAdapter>();
    FakeHardwareAdapter* adapterView = adapter.get();
    adapterView->frontObstacle = true;
    adapterView->sensors = rvc::PeriodicSensorData{
        .leftObstacle = false,
        .dustDetected = false,
    };

    rvc::Rvc rvc(std::move(adapter));
    rvc.startCleaning();

    const rvc::Command command = rvc.tick();

    EXPECT_TRUE(rvc.lastFrontObstacleInterrupt());
    EXPECT_FALSE(rvc.lastPeriodicSensors().leftObstacle);
    EXPECT_EQ(command.motion, rvc::Motion::TurnLeft);
    EXPECT_EQ(command.cleaningPower, rvc::CleaningPower::Normal); // R3: 회전 중에도 클리너 온 유지
    EXPECT_EQ(adapterView->applyCount, 1);
    EXPECT_EQ(adapterView->appliedCommand.motion, rvc::Motion::TurnLeft);
}

TEST(RvcTest, StopCleaningStillAppliesIdleCommandThroughAdapter) {
    auto adapter = std::make_unique<FakeHardwareAdapter>();
    FakeHardwareAdapter* adapterView = adapter.get();

    rvc::Rvc rvc(std::move(adapter));
    rvc.startCleaning();
    rvc.stopCleaning();

    const rvc::Command command = rvc.tick();

    EXPECT_EQ(command.motion, rvc::Motion::Stop);
    EXPECT_EQ(command.cleaningPower, rvc::CleaningPower::Off);
    EXPECT_EQ(adapterView->applyCount, 1);
    EXPECT_EQ(adapterView->appliedCommand.motion, rvc::Motion::Stop);
}

}  // namespace
