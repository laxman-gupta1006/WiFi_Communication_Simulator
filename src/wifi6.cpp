#include "../include/wifi6.h"
#include <numeric>
#include <thread>
#include <chrono>
#include <iostream>

WiFi6User::WiFi6User(int userId) : WiFi5User(userId) {}

std::unique_ptr<Packet> WiFi6User::createPacket() {
    return std::make_unique<Packet>(1024, id, 0);
}

bool WiFi6User::canTransmit() {
    return true;
}

WiFi6AccessPoint::WiFi6AccessPoint(int apId) 
    : AccessPoint(apId), CHANNEL_ALLOCATION_TIME(5.0), subChannelSizes({2, 4, 10}) {}

void WiFi6AccessPoint::simulateTransmission() {
    double currentTime = 0.0;
    const double SIMULATION_TIME = 1000.0; // 1 second
    int userIndex = 0;
    
    // Convert users to WiFi6Users
    std::vector<WiFi6User*> wifi6Users;
    for (auto& user : users) {
        if (auto wifi6User = dynamic_cast<WiFi6User*>(user.get())) {
            wifi6Users.push_back(wifi6User);
        }
    }
    
    while (currentTime < SIMULATION_TIME) {
        // Round-robin sub-channel allocation
        std::vector<std::pair<WiFi6User*, int>> allocations;
        
        for (auto user : wifi6Users) {
            if (user->canTransmit()) {
                int subChannelBW = subChannelSizes[userIndex % subChannelSizes.size()];
                allocations.push_back({user, subChannelBW});
                userIndex++;
            }
        }
        
        // Parallel transmission for 5ms window
        for (auto& allocation : allocations) {
            WiFi6User* user = allocation.first;
            int subChannelBW = allocation.second;
            
            auto packet = user->createPacket();
            
            // Calculate actual transmission time based on sub-channel bandwidth
            double actualTxTime = packet->calculateTransmissionTime(subChannelBW, 8, 5.0/6.0);
            
            // But transmission happens within the 5ms window
            packet->setTransmissionTime(currentTime, currentTime + CHANNEL_ALLOCATION_TIME);
            user->addTransmittedPacket(*packet);
            transmittedPackets.push_back(std::move(packet));
        }
        
        currentTime += CHANNEL_ALLOCATION_TIME;
    }
}

double WiFi6AccessPoint::computeThroughput() {
    double totalBits = 0.0;
    for (const auto& packet : transmittedPackets) {
        totalBits += packet->getSize() * 8;
    }
    return totalBits / 1000000.0; // Mbps (1 second simulation)
}

std::pair<double, double> WiFi6AccessPoint::computeLatency() {
    if (transmittedPackets.empty()) return {0.0, 0.0};
    
    double totalLatency = 0.0;
    double maxLatency = 0.0;
    
    for (const auto& packet : transmittedPackets) {
        double latency = packet->getLatency();
        totalLatency += latency;
        maxLatency = std::max(maxLatency, latency);
    }
    
    return {totalLatency / transmittedPackets.size(), maxLatency};
}
