#include "rvc/GridSimulator.hpp"

#include <exception>
#include <iostream>
#include <string>

namespace {

void printUsage(const char* executable) {
    std::cout << "Usage: " << executable << " [--ticks N] [--scenario FILE] [--quiet-map]\n";
}

}  // namespace

int main(int argc, char** argv) {
    try {
        int ticks = 20;
        bool ticksSetByUser = false;
        bool quietMap = false;
        std::string scenarioPath;

        for (int index = 1; index < argc; ++index) {
            const std::string arg = argv[index];
            if (arg == "--help" || arg == "-h") {
                printUsage(argv[0]);
                return 0;
            }

            if (arg == "--quiet-map") {
                quietMap = true;
                continue;
            }

            if (arg == "--ticks" && index + 1 < argc) {
                ticks = std::stoi(argv[++index]);
                ticksSetByUser = true;
                continue;
            }

            if (arg == "--scenario" && index + 1 < argc) {
                scenarioPath = argv[++index];
                continue;
            }

            std::cerr << "Unknown or incomplete argument: " << arg << '\n';
            printUsage(argv[0]);
            return 2;
        }

        rvc::GridSimulator simulator = [&]() {
            if (!scenarioPath.empty()) {
                rvc::Scenario scenario = rvc::GridSimulator::loadScenario(scenarioPath);
                if (!ticksSetByUser) {
                    ticks = scenario.ticks;
                }
                return rvc::GridSimulator::fromLines(scenario.mapLines);
            }

            return rvc::GridSimulator::fromLines(rvc::GridSimulator::defaultMap());
        }();

        if (!quietMap) {
            std::cout << "Initial map\n" << simulator.render() << '\n';
        }

        const rvc::SimulationResult result = simulator.run(ticks, !quietMap);

        for (const auto& line : result.logs) {
            std::cout << line << '\n';
        }

        std::cout << "Summary ticks=" << result.ticksRun
                  << " dustCleaned=" << result.dustCleaned
                  << " remainingDust=" << simulator.remainingDust()
                  << " finalPosition=(" << result.finalPosition.row << "," << result.finalPosition.col << ")"
                  << " finalDirection=" << rvc::toString(result.finalDirection) << '\n';

        if (!quietMap) {
            std::cout << "Final map\n" << simulator.render() << '\n';
        }

        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}
