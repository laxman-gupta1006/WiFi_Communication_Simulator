#include <iostream>
#include <vector>
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

// Template function to run the simulation for a given WiFi type
template <typename WiFiType>
void runWiFiSimulation(int users, Result& result, std::string wifiName) {
    int simulationIterations = 100;
    try {
        auto ap = std::make_unique<WiFiType>(1);
        for (int i = 0; i < users; ++i) {
            ap->addUser(std::make_unique<typename WiFiType::UserType>(i));
        }
        for (int iter = 0; iter < simulationIterations; ++iter) {
            ap->simulateTransmission();
        }
        
        if (wifiName == "WiFi4") {
            result.wifi4Throughput = ap->computeThroughput();
            auto [avgLatency, maxLatency] = ap->computeLatency();
            result.wifi4AvgLatency = avgLatency;
            result.wifi4MaxLatency = maxLatency;
        } else if (wifiName == "WiFi5") {
            result.wifi5Throughput = ap->computeThroughput();
            auto [avgLatency, maxLatency] = ap->computeLatency();
            result.wifi5AvgLatency = avgLatency;
            result.wifi5MaxLatency = maxLatency;
        } else if (wifiName == "WiFi6") {
            result.wifi6Throughput = ap->computeThroughput();
            auto [avgLatency, maxLatency] = ap->computeLatency();
            result.wifi6AvgLatency = avgLatency;
            result.wifi6MaxLatency = maxLatency;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error during " << wifiName << " simulation: " << e.what() << std::endl;
    }
}

// Function to run the simulation for different WiFi types and store results
void runSimulation(std::vector<Result>& results) {
    int simulationIterations = 100;
    std::vector<int> userScenarios = {1, 10, 100};

    for (int users : userScenarios) {
        std::cout << "\n===== Simulation with " << users << " Users =====\n";
        Result result = {users, 0, 0, 0, 0, 0, 0, 0, 0};

        // Run simulation for WiFi4, WiFi5, and WiFi6 using templates
        runWiFiSimulation<WiFi4AccessPoint>(users, result, "WiFi4");
        runWiFiSimulation<WiFi5AccessPoint>(users, result, "WiFi5");
        runWiFiSimulation<WiFi6AccessPoint>(users, result, "WiFi6");

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
