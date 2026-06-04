#include "Simulator/SystemTestMode.h"

#include "Simulator/RvcSimulator.h"
#include "Simulator/SystemTestCases.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <cctype>

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

void RunSystemTestMode(RvcSimulator& simulator) {
    simulator.setAutoStepSleepEnabled(false);

    const std::vector<SystemTestCase> testCases = BuildSystemTestCases();
    std::cout << "\nSystem Test Mode (1~30, A: 전체, Q: 종료)\n";

    while (true) {
        std::cout << "test> ";
        std::string input;
        if (!std::getline(std::cin, input)) {
            return;
        }

        input = Trim(input);
        if (input == "Q" || input == "q" || input == "quit") {
            return;
        }

        int passCount = 0;
        int failCount = 0;

        if (input == "A" || input == "a" || input == "all") {
            for (const auto& testCase : testCases) {
            simulator.setStepTraceEnabled(false);
                const SystemTestResult result = testCase.run(simulator);
                if (result.passed) {
                    ++passCount;
                    std::cout << "[Case " << testCase.id << "][" << testCase.type << "] PASS - " << testCase.name << "\n";
                } else {
                    ++failCount;
                    std::cout << "[Case " << testCase.id << "][" << testCase.type << "] FAIL - " << result.reason << "\n";
                }
            }
            std::cout << "Total: " << (passCount + failCount) << ", Pass: " << passCount << ", Fail: " << failCount << "\n";
            continue;
        }

        std::istringstream parser(input);
        int targetId = 0;
        parser >> targetId;
        if (parser.fail() || targetId < 1 || targetId > 30) {
            std::cout << "1~30 또는 A/Q를 입력하세요.\n";
            continue;
        }

        bool found = false;
        for (const auto& testCase : testCases) {
            if (testCase.id != targetId) {
                continue;
            }
            found = true;
            std::cout << "---- Case " << testCase.id << " Start ----\n";
            std::cout << "[" << testCase.type << "] " << testCase.name << "\n";
            simulator.setStepTraceEnabled(true);
            const SystemTestResult result = testCase.run(simulator);
            simulator.setStepTraceEnabled(false);
            std::cout << "---- Case " << testCase.id << " End ----\n";
            if (result.passed) {
                std::cout << "[Case " << testCase.id << "][" << testCase.type << "] PASS - " << testCase.name << "\n";
                std::cout << "Total: 1, Pass: 1, Fail: 0\n";
            } else {
                std::cout << "[Case " << testCase.id << "][" << testCase.type << "] FAIL - " << result.reason << "\n";
                std::cout << "Total: 1, Pass: 0, Fail: 1\n";
            }
            break;
        }

        if (!found) {
            std::cout << "해당 케이스를 찾을 수 없습니다.\n";
        }
    }
}
