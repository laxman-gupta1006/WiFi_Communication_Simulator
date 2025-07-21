#include <iostream>
#include <vector>
#include <iomanip>
#include <memory>

#include "../include/ap.h"
#include "../include/wifi4.h"
#include "../include/wifi5.h"
#include "../include/wifi6.h"

struct Result {
    int users;
    double wifi4Throughput, wifi5Throughput, wifi6Throughput;        // Mbps
    double wifi4AvgLatency, wifi5AvgLatency, wifi6AvgLatency;        // ms
    double wifi4MaxLatency, wifi5MaxLatency, wifi6MaxLatency;        // ms
};

void runSimulation(std::vector<Result>& results) {
    std::vector<int> userScenarios = {1, 10, 100};

    for (int numUsers : userScenarios) {
        std::cout << "\n===== Simulation with " << numUsers << " Users =====\n";
        Result result = {numUsers, 0, 0, 0, 0, 0, 0, 0, 0, 0};

        // WiFi 4 Simulation
        {
            std::cout << "Running WiFi 4 (CSMA/CA) simulation...\n";
            auto ap = std::make_unique<WiFi4AccessPoint>(1);
            for (int i = 0; i < numUsers; ++i) {
                ap->addUser(std::make_unique<WiFi4User>(i));
            }
            ap->simulateTransmission();
            result.wifi4Throughput = ap->computeThroughput();
            auto [avgLat, maxLat] = ap->computeLatency();
            result.wifi4AvgLatency = avgLat;
            result.wifi4MaxLatency = maxLat;
            std::cout << "  Throughput: " << std::fixed << std::setprecision(2) 
                      << result.wifi4Throughput << " Mbps\n";
            std::cout << "  Avg Latency: " << result.wifi4AvgLatency << " ms\n";
            std::cout << "  Max Latency: " << result.wifi4MaxLatency << " ms\n";
        }

        // WiFi 5 Simulation
        {
            std::cout << "Running WiFi 5 (MU-MIMO) simulation...\n";
            auto ap = std::make_unique<WiFi5AccessPoint>(1);
            for (int i = 0; i < numUsers; ++i) {
                ap->addUser(std::make_unique<WiFi5User>(i));
            }
            ap->simulateTransmission();
            result.wifi5Throughput = ap->computeThroughput();
            auto [avgLat, maxLat] = ap->computeLatency();
            result.wifi5AvgLatency = avgLat;
            result.wifi5MaxLatency = maxLat;
            std::cout << "  Throughput: " << std::fixed << std::setprecision(2) 
                      << result.wifi5Throughput << " Mbps\n";
            std::cout << "  Avg Latency: " << result.wifi5AvgLatency << " ms\n";
            std::cout << "  Max Latency: " << result.wifi5MaxLatency << " ms\n";
        }

        // WiFi 6 Simulation
        {
            std::cout << "Running WiFi 6 (OFDMA) simulation...\n";
            auto ap = std::make_unique<WiFi6AccessPoint>(1);
            for (int i = 0; i < numUsers; ++i) {
                ap->addUser(std::make_unique<WiFi6User>(i));
            }
            ap->simulateTransmission();
            result.wifi6Throughput = ap->computeThroughput();
            auto [avgLat, maxLat] = ap->computeLatency();
            result.wifi6AvgLatency = avgLat;
            result.wifi6MaxLatency = maxLat;
            std::cout << "  Throughput: " << std::fixed << std::setprecision(2) 
                      << result.wifi6Throughput << " Mbps\n";
            std::cout << "  Avg Latency: " << result.wifi6AvgLatency << " ms\n";
            std::cout << "  Max Latency: " << result.wifi6MaxLatency << " ms\n";
        }

        results.push_back(result);
    }
}

void printResults(const std::vector<Result>& results) {
    std::cout << "\n" << std::string(120, '=') << "\n";
    std::cout << "               WiFi Communication Simulation Results Summary\n";
    std::cout << std::string(120, '=') << "\n";
    
    // Header with units
    std::cout << std::left 
              << std::setw(8) << "Users"
              << std::setw(15) << "WiFi4 Tput"
              << std::setw(15) << "WiFi4 AvgLat"
              << std::setw(15) << "WiFi4 MaxLat"
              << std::setw(15) << "WiFi5 Tput"
              << std::setw(15) << "WiFi5 AvgLat"
              << std::setw(15) << "WiFi5 MaxLat"
              << std::setw(15) << "WiFi6 Tput"
              << std::setw(15) << "WiFi6 AvgLat"
              << std::setw(15) << "WiFi6 MaxLat"
              << "\n";
    
    // Units row
    std::cout << std::left 
              << std::setw(8) << ""
              << std::setw(15) << "(Mbps)"
              << std::setw(15) << "(ms)"
              << std::setw(15) << "(ms)"
              << std::setw(15) << "(Mbps)"
              << std::setw(15) << "(ms)"
              << std::setw(15) << "(ms)"
              << std::setw(15) << "(Mbps)"
              << std::setw(15) << "(ms)"
              << std::setw(15) << "(ms)"
              << "\n";
    
    std::cout << std::string(120, '-') << "\n";

    for (const auto& result : results) {
        std::cout << std::left 
                  << std::setw(8) << result.users
                  << std::setw(15) << std::fixed << std::setprecision(2) << result.wifi4Throughput
                  << std::setw(15) << std::fixed << std::setprecision(3) << result.wifi4AvgLatency
                  << std::setw(15) << std::fixed << std::setprecision(3) << result.wifi4MaxLatency
                  << std::setw(15) << std::fixed << std::setprecision(2) << result.wifi5Throughput
                  << std::setw(15) << std::fixed << std::setprecision(3) << result.wifi5AvgLatency
                  << std::setw(15) << std::fixed << std::setprecision(3) << result.wifi5MaxLatency
                  << std::setw(15) << std::fixed << std::setprecision(2) << result.wifi6Throughput
                  << std::setw(15) << std::fixed << std::setprecision(3) << result.wifi6AvgLatency
                  << std::setw(15) << std::fixed << std::setprecision(3) << result.wifi6MaxLatency
                  << "\n";
    }
    
    std::cout << std::string(120, '-') << "\n";
    
    // Add simulation parameters
    std::cout << "\nSimulation Parameters:\n";
    std::cout << "• Bandwidth: 20 MHz\n";
    std::cout << "• Modulation: 256-QAM (8 bits/symbol)\n";
    std::cout << "• Coding Rate: 5/6\n";
    std::cout << "• Packet Size: 1024 bytes (1 KB)\n";
    std::cout << "• WiFi 5 CSI Packet Size: 200 bytes\n";
    std::cout << "• WiFi 5 Parallel Window: 15 ms\n";
    std::cout << "• WiFi 6 Sub-channels: 2, 4, 10 MHz\n";
    std::cout << "• WiFi 6 Allocation Window: 5 ms\n";
    std::cout << "• Simulation Duration: 1000 ms (1 second)\n";
}

void printDetailedAnalysis(const std::vector<Result>& results) {
    std::cout << "\n" << std::string(80, '=') << "\n";
    std::cout << "                    Performance Analysis\n";
    std::cout << std::string(80, '=') << "\n";
    
    for (const auto& result : results) {
        std::cout << "\n--- " << result.users << " User" << (result.users > 1 ? "s" : "") << " ---\n";
        
        // Throughput comparison
        std::cout << "Throughput Comparison:\n";
        std::cout << "  WiFi 4 (CSMA/CA): " << std::fixed << std::setprecision(2) 
                  << result.wifi4Throughput << " Mbps\n";
        std::cout << "  WiFi 5 (MU-MIMO): " << result.wifi5Throughput << " Mbps";
        if (result.wifi5Throughput > result.wifi4Throughput) {
            std::cout << " (+" << std::setprecision(1) 
                      << ((result.wifi5Throughput - result.wifi4Throughput) / result.wifi4Throughput * 100) 
                      << "% improvement)";
        }
        std::cout << "\n";
        
        std::cout << "  WiFi 6 (OFDMA):   " << result.wifi6Throughput << " Mbps";
        if (result.wifi6Throughput > result.wifi4Throughput) {
            std::cout << " (+" << std::setprecision(1) 
                      << ((result.wifi6Throughput - result.wifi4Throughput) / result.wifi4Throughput * 100) 
                      << "% improvement)";
        }
        std::cout << "\n";
        
        // Latency comparison
        std::cout << "\nAverage Latency Comparison:\n";
        std::cout << "  WiFi 4: " << std::fixed << std::setprecision(3) 
                  << result.wifi4AvgLatency << " ms\n";
        std::cout << "  WiFi 5: " << result.wifi5AvgLatency << " ms\n";
        std::cout << "  WiFi 6: " << result.wifi6AvgLatency << " ms\n";
        
        // Best performer
        double bestThroughput = std::max({result.wifi4Throughput, result.wifi5Throughput, result.wifi6Throughput});
        std::string bestProtocol = (bestThroughput == result.wifi4Throughput) ? "WiFi 4" :
                                  (bestThroughput == result.wifi5Throughput) ? "WiFi 5" : "WiFi 6";
        std::cout << "\nBest Throughput: " << bestProtocol << " with " 
                  << std::fixed << std::setprecision(2) << bestThroughput << " Mbps\n";
    }
}

int main() {
    std::vector<Result> results;
    
    std::cout << std::string(60, '=') << "\n";
    std::cout << "        WiFi Communication Simulator\n";
    std::cout << "   Comparing WiFi 4, 5, and 6 protocols\n";
    std::cout << std::string(60, '=') << "\n";
    
    std::cout << "\nStarting simulations...\n";
    
    runSimulation(results);
    printResults(results);
    printDetailedAnalysis(results);
    
    std::cout << "\nSimulation completed successfully!\n";
    std::cout << std::string(60, '=') << "\n";
    
    return 0;
}
