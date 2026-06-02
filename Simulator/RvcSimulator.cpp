#include "Simulator/RvcSimulator.h"
#include "Simulator/SystemTestMode.h"

#include <chrono>
#include <cctype>
#include <iostream>
#include <sstream>
#include <thread>

namespace {
std::string Trim(const std::string& value) {
    size_t start = 0;
    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start]))) {
        ++start;
    }
    size_t end = value.size();
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
        --end;
    }
    return value.substr(start, end - start);
}
}  // namespace

RvcSimulator::RvcSimulator()
    : motor(&map),
      frontSensor(&map, &motor, SensorDirection::Front),
      leftSensor(&map, &motor, SensorDirection::Left),
      dustSensor(&map, &motor, SensorDirection::Dust),
      cleaner(&map, &motor),
      sensorController(&bus, &leftSensor, &dustSensor),
      cleanerController(&bus, &cleaner, &cleanerTimer),
      motorController(&bus, motor),
      powerController(&bus),
      powerOn(false),
      autoStepSleepEnabled(true),
      consecutiveAvoidSteps(0),
      stepTraceEnabled(false) {
}

RvcSimulator::~RvcSimulator() {
    turnOff();
}

void RvcSimulator::run() {
    while (true) {
        std::cout << "\n===== RVC Simulator =====\n";
        std::cout << "1. 기본 모드\n";
        std::cout << "2. 테스트 모드\n";
        std::cout << "3. 종료\n";
        std::cout << "선택> ";

        std::string line;
        if (!std::getline(std::cin, line)) {
            return;
        }

        const std::string menu = Trim(line);
        if (menu == "1") {
            runFreeMode();
        } else if (menu == "2") {
            RunSystemTestMode(*this);
        } else if (menu == "3" || menu == "q" || menu == "quit") {
            return;
        } else {
            std::cout << "잘못된 입력입니다.\n";
        }
    }
}

void RvcSimulator::runFreeMode() {
    std::cout << "RVC CLI 격자 시뮬레이터\n";
    printHelp();
    printScreen();

    std::string line;
    while (true) {
        std::cout << "\n명령> ";
        if (!std::getline(std::cin, line)) {
            break;
        }
        if (!handleCommand(line)) {
            break;
        }
    }
}

void RvcSimulator::reset() {
    turnOff();
    map.resetDefault();
    motor.resetPose(Point(1, 1), Point(0, 1));
    cleaner.reset();
    powerOn = false;
    setSensorFault(SensorDirection::Front, SimulatedSensor::FaultMode::Normal);
    setSensorFault(SensorDirection::Left, SimulatedSensor::FaultMode::Normal);
    setSensorFault(SensorDirection::Right, SimulatedSensor::FaultMode::Normal);
    setSensorFault(SensorDirection::Dust, SimulatedSensor::FaultMode::Normal);
    setMotorBroken(false);
    consecutiveAvoidSteps = 0;
}

void RvcSimulator::resetSystemTestMap() {
    turnOff();
    map.resetSystemTestBase();
    motor.resetPose(Point(1, 1), Point(0, 1));
    cleaner.reset();
    powerOn = false;
    setSensorFault(SensorDirection::Front, SimulatedSensor::FaultMode::Normal);
    setSensorFault(SensorDirection::Left, SimulatedSensor::FaultMode::Normal);
    setSensorFault(SensorDirection::Right, SimulatedSensor::FaultMode::Normal);
    setSensorFault(SensorDirection::Dust, SimulatedSensor::FaultMode::Normal);
    setMotorBroken(false);
    consecutiveAvoidSteps = 0;
}

void RvcSimulator::resetRandomMap(uint32_t seed) {
    turnOff();
    map.resetRandom(seed);
    motor.resetPose(Point(1, 1), Point(0, 1));
    cleaner.reset();
    powerOn = false;
}

void RvcSimulator::turnOn() {
    if (powerOn) {
        return;
    }

    powerOn = true;
    sensorController.turnOn();
    motorController.turnOn();
    powerController.turnOn();
    sensorController.ChecknPowerUp();
    cleaner.cleanCurrentCell();
    bus.publishStartCleaning();
}

void RvcSimulator::turnOff() {
    if (!powerOn) {
        cleaner.reset();
        return;
    }

    powerOn = false;
    sensorController.turnOff();
    motorController.turnOff();
    powerController.turnOff();
}

void RvcSimulator::step() {
    if (!powerOn) {
        if (stepTraceEnabled) {
            printScreen();
        }
        return;
    }

    motor.clearBlocked();
    Point facingBeforeObstacleCheck = motor.getFacing();
    bool obstacleEventHandled = false;
    if (frontSensor.detect() && !motorController.isAvoiding()) {
        sensorController.FrontObstacleDetected();
        obstacleEventHandled = true;
    }
    if (!(obstacleEventHandled && !facingBeforeObstacleCheck.isEqual(motor.getFacing()))) {
        motorController.MCMove();
    }
    if (motorController.isAvoiding()) {
        ++consecutiveAvoidSteps;
    } else {
        consecutiveAvoidSteps = 0;
    }

    sensorController.ChecknPowerUp();
    cleaner.cleanCurrentCell();
    cleanerTimer.syncTimerDigitalClock();

    if (stepTraceEnabled) {
        printScreen();
    }
}

void RvcSimulator::autoStep(int count) {
    if (count < 0) {
        return;
    }

    for (int i = 0; i < count; ++i) {
        step();
        printScreen();
        if (autoStepSleepEnabled) {
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
        }
    }
}

void RvcSimulator::forceFrontObstacle() {
    if (powerOn) {
        sensorController.FrontObstacleDetected();
        sensorController.ChecknPowerUp();
        cleaner.cleanCurrentCell();
    }
}

bool RvcSimulator::addDust(int x, int y) {
    return map.addDust(Point(x, y));
}

bool RvcSimulator::toggleWall(int x, int y) {
    Point point(x, y);
    if (motor.getPosition().isEqual(point)) {
        return false;
    }

    return map.toggleWall(point);
}

std::string RvcSimulator::statusText() const {
    std::ostringstream out;
    out << "전원: " << (powerOn ? "ON" : "OFF")
        << " | 청소기: " << (cleaner.isTurnedOn() ? "ON" : "OFF")
        << " | 파워업: " << (cleaner.isPowerUp() ? "ON" : "OFF") << '\n';
    out << "위치: (" << motor.getPosition().x << ", " << motor.getPosition().y << ")"
        << " | 방향: " << directionText()
        << " (" << motor.getFacing().x << ", " << motor.getFacing().y << ")\n";
    out << "센서 - 전방벽: " << (frontSensor.peek() ? "1" : "0")
        << ", 좌측벽: " << (leftSensor.peek() ? "1" : "0")
        << ", 우측벽: " << ("모르는 상태")
        << ", 현재먼지: " << (dustSensor.peek() ? "1" : "0") << '\n';
    out << "범례: # 벽, . 빈칸, * 먼지, x 청소완료, ^v<> RVC";
    return out.str();
}

std::string RvcSimulator::render() const {
    std::ostringstream out;
    for (const auto& line : map.render(motor.getPosition(), motor.getFacing())) {
        out << line << '\n';
    }
    return out.str();
}

Point RvcSimulator::getRobotPoint() const {
    return motor.getPosition();
}

Point RvcSimulator::getRobotDirection() const {
    return motor.getFacing();
}

bool RvcSimulator::isPowerOn() const {
    return powerOn;
}

bool RvcSimulator::isCleanerOn() const {
    return cleaner.isTurnedOn();
}

bool RvcSimulator::isPowerUp() const {
    return cleaner.isPowerUp();
}

bool RvcSimulator::isAvoiding() const {
    return motorController.isAvoiding();
}

int RvcSimulator::getConsecutiveAvoidSteps() const {
    return consecutiveAvoidSteps;
}

bool RvcSimulator::isWallAt(int x, int y) const {
    return map.isWall(Point(x, y));
}

bool RvcSimulator::loadSystemTestScenario(const std::vector<std::string>& layoutRowsTopToBottom,
                                          Point startPoint,
                                          Point startDirection) {
    turnOff();
    if (!map.loadLayout(layoutRowsTopToBottom)) {
        return false;
    }
    if (!map.isInside(startPoint) || map.isWall(startPoint)) {
        return false;
    }

    motor.resetPose(startPoint, startDirection);
    cleaner.reset();
    powerOn = false;
    setSensorFault(SensorDirection::Front, SimulatedSensor::FaultMode::Normal);
    setSensorFault(SensorDirection::Left, SimulatedSensor::FaultMode::Normal);
    setSensorFault(SensorDirection::Right, SimulatedSensor::FaultMode::Normal);
    setSensorFault(SensorDirection::Dust, SimulatedSensor::FaultMode::Normal);
    setMotorBroken(false);
    consecutiveAvoidSteps = 0;
    return true;
}

void RvcSimulator::setSensorFault(SensorDirection direction, SimulatedSensor::FaultMode mode) {
    if (direction == SensorDirection::Front) {
        frontSensor.setFaultMode(mode);
    } else if (direction == SensorDirection::Left) {
        leftSensor.setFaultMode(mode);
    // } else if (direction == SensorDirection::Right) {
    //     rightSensor.setFaultMode(mode);
    } else {
        dustSensor.setFaultMode(mode);
    }
}

void RvcSimulator::setMotorBroken(bool broken) {
    motor.setBroken(broken);
}

bool RvcSimulator::isMotorBlocked() const {
    return motor.wasBlocked();
}

void RvcSimulator::setAutoStepSleepEnabled(bool enabled) {
    autoStepSleepEnabled = enabled;
}

void RvcSimulator::setStepTraceEnabled(bool enabled) {
    stepTraceEnabled = enabled;
}

void RvcSimulator::showCurrentScreen() const {
    printScreen();
}

void RvcSimulator::printScreen() const {
    std::cout << '\n' << render();
    std::cout << statusText() << '\n';
}

void RvcSimulator::printHelp() const {
    std::cout
        << "\n명령 목록\n"
        << "  on              전원 켜기\n"
        << "  off             전원 끄기\n"
        << "  step            1틱 진행\n"
        << "  auto N          N틱 자동 진행\n"
        << "  front           전방 장애물 이벤트 강제 발생\n"
        << "  dust x y        지정 칸에 먼지 추가\n"
        << "  wall x y        지정 칸 벽 토글\n"
        << "  status          현재 맵과 상태 출력\n"
        << "  reset           기본 맵으로 초기화\n"
        << "  randmap [시드]   무작위 맵(시드 생략 시 매번 다름)\n"
        << "  help            도움말 출력\n"
        << "  quit            종료\n";
}

bool RvcSimulator::handleCommand(const std::string& line) {
    std::istringstream input(line);
    std::string command;
    input >> command;

    if (command.empty() || command == "status") {
        printScreen();
    } else if (command == "help") {
        printHelp();
    } else if (command == "on") {
        turnOn();
        printScreen();
    } else if (command == "off") {
        turnOff();
        printScreen();
    } else if (command == "step") {
        step();
        printScreen();
    } else if (command == "auto") {
        int count = 0;
        input >> count;
        autoStep(count);
    } else if (command == "front") {
        forceFrontObstacle();
        printScreen();
    } else if (command == "dust") {
        int x = 0;
        int y = 0;
        if (input >> x >> y && addDust(x, y)) {
            printScreen();
        } else {
            std::cout << "먼지를 놓을 수 없는 위치입니다.\n";
        }
    } else if (command == "wall") {
        int x = 0;
        int y = 0;
        if (input >> x >> y && toggleWall(x, y)) {
            printScreen();
        } else {
            std::cout << "벽을 변경할 수 없는 위치입니다.\n";
        }
    } else if (command == "randmap") {
        uint32_t seed = 0;
        if (input >> seed) {
            resetRandomMap(seed);
        } else {
            resetRandomMap(0);
        }
        printScreen();
    } else if (command == "reset") {
        reset();
        printScreen();
    } else if (command == "quit" || command == "exit") {
        return false;
    } else {
        std::cout << "알 수 없는 명령입니다. help를 입력해 주세요.\n";
    }

    return true;
}

std::string RvcSimulator::directionText() const {
    Point d = motor.getFacing();
    if (d.x == 0 && d.y == -1) return "아래";
    if (d.x == 0 && d.y == 1) return "위";
    if (d.x == -1 && d.y == 0) return "왼쪽";
    if (d.x == 1 && d.y == 0) return "오른쪽";
    return "알수없음";
}
