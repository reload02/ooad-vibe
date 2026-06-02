#pragma once

#include <functional>
#include <string>
#include <vector>

class RvcSimulator;

struct SystemTestResult {
    bool passed;
    std::string reason;
};

struct SystemTestCase {
    int id;
    std::string type;
    std::string name;
    std::function<SystemTestResult(RvcSimulator&)> run;
};

std::vector<SystemTestCase> BuildSystemTestCases();
