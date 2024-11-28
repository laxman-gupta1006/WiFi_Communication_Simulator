#include "../include/wifi5.h"
#include <numeric>

#include <thread>   // For std::thread
#include <vector>   // For std::vector
#include <numeric>  // If you need std::accumulate (already included)
#include <chrono>   // For std::chrono::microseconds (if used)
#include <iostream> // Optional for debugging

constexpr double PARALLEL_TIME_MS = 5.0; 

WiFi5User::WiFi5User(int userId) : WiFi4User(userId), hasChannelState(false) {}

std::unique_ptr<Packet> WiFi5User::createPacket() {
    return std::make_unique<Packet>(200, id, rand() % 100);
}

bool WiFi5User::canTransmit() {
    return hasChannelState;
}

 void WiFi5User::setChannelState(bool state) {
        hasChannelState = state;
    }

// WiFi5AccessPoint Implementation
WiFi5AccessPoint::WiFi5AccessPoint(int apId) : AccessPoint(apId), PARALLEL_TIME(15.0) {}


void WiFi5AccessPoint::simulateTransmission() {
    std::vector<std::thread> userThreads;

    for (auto& user : users) {
        userThreads.emplace_back([&, userPtr = user.get()]() {
            if (userPtr->canTransmit()) {
                auto startTime = std::chrono::high_resolution_clock::now(); // Start time
                
                // Simulate parallel transmissions with a realistic delay
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    auto packet = userPtr->createPacket();
                    transmittedPackets.push_back(std::move(packet));
                }

                auto endTime = std::chrono::high_resolution_clock::now(); // End time
                std::chrono::duration<double, std::milli> elapsed = endTime - startTime;

                // Simulate a constant parallel transmission time (e.g., 5ms for WiFi5)
                double computedLatency = elapsed.count() + PARALLEL_TIME_MS;
                {
                    std::lock_guard<std::mutex> latencyLock(mutex);
                    latencies.push_back(computedLatency);
                }
            }
        });
    }

    for (auto& thread : userThreads) {
        if (thread.joinable()) thread.join();
    }
}

std::pair<double, double> WiFi5AccessPoint::computeLatency() {
    std::lock_guard<std::mutex> lock(mutex);
    if (latencies.empty()) return {0.0, 0.0};

    double totalLatency = std::accumulate(latencies.begin(), latencies.end(), 0.0);
    double averageLatency = totalLatency / latencies.size();
    double maxLatency = *std::max_element(latencies.begin(), latencies.end());

    return {averageLatency, maxLatency};
}

double WiFi5AccessPoint::computeThroughput() {
    double totalDataBits = 0.0;
    for (const auto& packet : transmittedPackets) {
        totalDataBits += packet->getSize() * 8;
    }
    return totalDataBits / PARALLEL_TIME; // Throughput in bits/sec
}