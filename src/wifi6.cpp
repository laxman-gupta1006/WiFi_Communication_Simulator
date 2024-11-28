#include "../include/wifi6.h"
#include <numeric>
#include <thread>   // For std::thread
#include <vector>   // For std::vector
#include <numeric>  // If you need std::accumulate (already included)
#include <chrono>   // For std::chrono::microseconds (if used)
#include <iostream> // Optional for debugging

WiFi6User::WiFi6User(int userId) : WiFi5User(userId) {}

std::unique_ptr<Packet> WiFi6User::createPacket() {
    return std::make_unique<Packet>(1024, id, rand() % 100);
}

bool WiFi6User::canTransmit() {
    return true;
}

// WiFi6AccessPoint Implementation
WiFi6AccessPoint::WiFi6AccessPoint(int apId)
    : AccessPoint(apId), CHANNEL_ALLOCATION_TIME(5.0), subChannelSizes({2, 4, 10}) {}


double WiFi6AccessPoint::computeThroughput() {
    double totalDataBits = 0.0;
    for (const auto& packet : transmittedPackets) {
        totalDataBits += packet->getSize() * 8;
    }
    return totalDataBits / CHANNEL_ALLOCATION_TIME; // Throughput in bits/sec
}

void WiFi6AccessPoint::simulateTransmission() {
    std::vector<std::thread> userThreads;

    for (auto& user : users) {
        userThreads.emplace_back([&, userPtr = user.get()]() {
            if (userPtr->canTransmit()) {
                auto startTime = std::chrono::high_resolution_clock::now(); // Start time
                
                // Simulate sub-channel allocation for this user
                int subChannelSize = subChannelSizes[userPtr->getId() % subChannelSizes.size()];
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    auto packet = userPtr->createPacket();
                    transmittedPackets.push_back(std::move(packet));
                }

                auto endTime = std::chrono::high_resolution_clock::now(); // End time
                std::chrono::duration<double> elapsed = endTime - startTime;

                // Adjust latency based on allocated sub-channel size
                double adjustedLatency = elapsed.count() + CHANNEL_ALLOCATION_TIME / subChannelSize;
                {
                    std::lock_guard<std::mutex> latencyLock(mutex);
                    latencies.push_back(adjustedLatency);
                }
            }
        });
    }

    for (auto& thread : userThreads) {
        if (thread.joinable()) thread.join();
    }
}

std::pair<double, double> WiFi6AccessPoint::computeLatency() {
    std::lock_guard<std::mutex> lock(mutex);
    if (latencies.empty()) return {0.0, 0.0};
    
    double totalLatency = std::accumulate(latencies.begin(), latencies.end(), 0.0);
    double averageLatency = totalLatency / latencies.size();
    double maxLatency = *std::max_element(latencies.begin(), latencies.end());
    
    return {averageLatency, maxLatency};
}