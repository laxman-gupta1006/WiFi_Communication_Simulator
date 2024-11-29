#include <iostream>
#include <vector>
#include <iomanip>
#include <thread>
#include <mutex>
#include <numeric>
#include <chrono>
#include <iomanip>

#include "../include/ap.h"
#include "../include/packet.h"
#include "../include/user.h"
#include "../include/wifi4.h"
#include "../include/wifi5.h"
#include "../include/wifi6.h"

// Result structure to hold simulation results
struct Result {
    int users;
    long double wifi4Throughput, wifi5Throughput, wifi6Throughput;
    long double wifi4AvgLatency, wifi5AvgLatency, wifi6AvgLatency;
    long double wifi4MaxLatency, wifi5MaxLatency, wifi6MaxLatency;
};

// Function to run the simulation
void runSimulation(std::vector<Result>& results) {
    int simulationIterations = 100;
    std::vector<int> userScenarios = {1, 10, 100};

    for (int users : userScenarios) {
        std::cout << "\n===== Simulation with " << users << " Users =====\n";
        Result result = {users, 0, 0, 0, 0, 0, 0, 0, 0, 0};

        // WiFi4 Simulation
        {
            auto wifi4AP = std::make_unique<WiFi4AccessPoint>(1);
            for (int i = 0; i < users; ++i) {
                wifi4AP->addUser(std::make_unique<WiFi4User>(i));
            }
            for (int iter = 0; iter < simulationIterations; ++iter) {
                wifi4AP->simulateTransmission();
            }
            result.wifi4Throughput = wifi4AP->computeThroughput();
            auto [avgLatency, maxLatency] = wifi4AP->computeLatency();
            result.wifi4AvgLatency = avgLatency;
            result.wifi4MaxLatency = maxLatency;
        }

        // WiFi5 Simulation
        {
            auto wifi5AP = std::make_unique<WiFi5AccessPoint>(1);
            for (int i = 0; i < users; ++i) {
                auto wifi5User = std::make_unique<WiFi5User>(i);
                wifi5User->setChannelState(true);
                wifi5AP->addUser(std::move(wifi5User));
            }
            for (int iter = 0; iter < simulationIterations; ++iter) {
                wifi5AP->simulateTransmission();
            }
            result.wifi5Throughput = wifi5AP->computeThroughput();
            auto [avgLatency, maxLatency] = wifi5AP->computeLatency();
            result.wifi5AvgLatency = avgLatency;
            result.wifi5MaxLatency = maxLatency;
        }

        // WiFi6 Simulation
        {
            auto wifi6AP = std::make_unique<WiFi6AccessPoint>(1);
            for (int i = 0; i < users; ++i) {
                wifi6AP->addUser(std::make_unique<WiFi6User>(i));
            }
            for (int iter = 0; iter < simulationIterations; ++iter) {
                wifi6AP->simulateTransmission();
            }
            result.wifi6Throughput = wifi6AP->computeThroughput();
            auto [avgLatency, maxLatency] = wifi6AP->computeLatency();
            result.wifi6AvgLatency = avgLatency;
            result.wifi6MaxLatency = maxLatency;
        }

        results.push_back(result);
    }
}

// Function to print the simulation results
void printResults(const std::vector<Result>& results) {
    std::cout << "\n===== Simulation Results Comparison Table =====\n";
    std::cout << std::left << std::setw(10) << "Users"
              << std::setw(15) << "WiFi4 Thrpt"
              << std::setw(15) << "WiFi4 Avg Lat"
              << std::setw(15) << "WiFi4 Max Lat"
              << std::setw(15) << "WiFi5 Thrpt"
              << std::setw(15) << "WiFi5 Avg Lat"
              << std::setw(15) << "WiFi5 Max Lat"
              << std::setw(15) << "WiFi6 Thrpt"
              << std::setw(15) << "WiFi6 Avg Lat"
              << std::setw(15) << "WiFi6 Max Lat"
              << "\n";
    std::cout << std::string(140, '-') << "\n";

    for (const auto& result : results) {
        std::cout << std::left << std::setw(10) << result.users
                  << std::setw(15) << result.wifi4Throughput
                  << std::setw(15) << result.wifi4AvgLatency
                  << std::setw(15) << result.wifi4MaxLatency
                  << std::setw(15) << result.wifi5Throughput
                  << std::setw(15) << result.wifi5AvgLatency
                  << std::setw(15) << result.wifi5MaxLatency
                  << std::setw(15) << result.wifi6Throughput
                  << std::setw(15) << result.wifi6AvgLatency
                  << std::setw(15) << result.wifi6MaxLatency
                  << "\n";
    }
}

int main() {
    std::vector<Result> results;
    int choice;

    do {
        std::cout << "\n===== Main Menu =====\n";
        std::cout << "1. Run Simulation\n";
        std::cout << "2. View Simulation Results\n";
        std::cout << "3. Exit\n";
        std::cout << "Enter your choice: ";
        std::cin >> choice;

        switch (choice) {
            case 1:
                std::cout << "Running simulation...\n";
                runSimulation(results);
                std::cout << "Simulation completed.\n";
                break;
            case 2:
                if (results.empty()) {
                    std::cout << "No results available. Run a simulation first.\n";
                } else {
                    printResults(results);
                }
                break;
            case 3:
                std::cout << "Exiting program.\n";
                break;
            default:
                std::cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != 3);

    return 0;
}
