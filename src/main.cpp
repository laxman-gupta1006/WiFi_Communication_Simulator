#include <iostream>
#include <vector>
#include <iomanip>
#include <thread>
#include <mutex>
#include <numeric>
#include <chrono>
#include <iomanip>
#include <memory>
#include <type_traits>

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

// Template function for running WiFi technology simulation
template <typename AccessPointType, typename UserType>
void runTechnologySimulation(Result& result, int users, int simulationIterations) {
    auto accessPoint = std::make_unique<AccessPointType>(1);
    
    // Add users
    for (int i = 0; i < users; ++i) {
        // Special handling for WiFi5 channel state
        if constexpr (std::is_same_v<AccessPointType, WiFi5AccessPoint>) {
            auto user = std::make_unique<UserType>(i);
            user->setChannelState(true);
            accessPoint->addUser(std::move(user));
        } else {
            accessPoint->addUser(std::make_unique<UserType>(i));
        }
    }

    // Run simulations
    for (int iter = 0; iter < simulationIterations; ++iter) {
        accessPoint->simulateTransmission();
    }

    // Compute metrics based on technology
    if constexpr (std::is_same_v<AccessPointType, WiFi4AccessPoint>) {
        result.wifi4Throughput = accessPoint->computeThroughput();
        auto [avgLatency, maxLatency] = accessPoint->computeLatency();
        result.wifi4AvgLatency = avgLatency;
        result.wifi4MaxLatency = maxLatency;
    } else if constexpr (std::is_same_v<AccessPointType, WiFi5AccessPoint>) {
        result.wifi5Throughput = accessPoint->computeThroughput();
        auto [avgLatency, maxLatency] = accessPoint->computeLatency();
        result.wifi5AvgLatency = avgLatency;
        result.wifi5MaxLatency = maxLatency;
    } else if constexpr (std::is_same_v<AccessPointType, WiFi6AccessPoint>) {
        result.wifi6Throughput = accessPoint->computeThroughput();
        auto [avgLatency, maxLatency] = accessPoint->computeLatency();
        result.wifi6AvgLatency = avgLatency;
        result.wifi6MaxLatency = maxLatency;
    }
}

// Function to run the simulation
void runSimulation(std::vector<Result>& results) {
    int simulationIterations = 100;
    std::vector<int> userScenarios = {1, 10, 100};

    for (int users : userScenarios) {
        std::cout << "\n===== Simulation with " << users << " Users =====\n";
        Result result = {users, 0, 0, 0, 0, 0, 0, 0, 0, 0};

        // Template-based simulation for WiFi technologies
        runTechnologySimulation<WiFi4AccessPoint, WiFi4User>(result, users, simulationIterations);
        runTechnologySimulation<WiFi5AccessPoint, WiFi5User>(result, users, simulationIterations);
        runTechnologySimulation<WiFi6AccessPoint, WiFi6User>(result, users, simulationIterations);

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
                break;
        }
    } while (choice != 3);

    return 0;
}
