#include "../include/wifi5.h"
#include <thread>
#include <chrono>

WiFi5User::WiFi5User(int userId) : WiFi4User(userId), hasChannelState(false) {}

std::unique_ptr<Packet> WiFi5User::createPacket() {
    return std::make_unique<Packet>(1024, id, 0); // 1KB data packet
}

bool WiFi5User::canTransmit() {
    return hasChannelState;
}

void WiFi5User::setChannelState(bool state) {
    hasChannelState = state;
}

bool WiFi5User::isInBeamformedRange() {
    return true; // Assume all users are in range
}

std::unique_ptr<Packet> WiFi5User::createChannelStatePacket(int size) {
    return std::make_unique<Packet>(size, id, 0);
}

WiFi5AccessPoint::WiFi5AccessPoint(int apId) 
    : AccessPoint(apId), PARALLEL_TIME(15.0) {}

void WiFi5AccessPoint::simulateTransmission() {
    double currentTime = 0.0;
    const double SIMULATION_TIME = 1000.0; // 1 second
    
    // Convert users to WiFi5Users
    std::vector<WiFi5User*> wifi5Users;
    for (auto& user : users) {
        if (auto wifi5User = dynamic_cast<WiFi5User*>(user.get())) {
            wifi5Users.push_back(wifi5User);
        }
    }
    
    while (currentTime < SIMULATION_TIME) {
        // Step 1: AP broadcasts packet
        auto broadcastPacket = std::make_unique<Packet>(1024, 0, -1); // Broadcast
        double broadcastTime = broadcastPacket->calculateTransmissionTime(20.0, 8, 5.0/6.0);
        broadcastPacket->setTransmissionTime(currentTime, currentTime + broadcastTime);
        transmittedPackets.push_back(std::move(broadcastPacket));
        currentTime += broadcastTime;
        
        // Step 2: Sequential channel state information
        for ( auto user : wifi5Users) {
            auto csiPacket = user->createChannelStatePacket(200); // 200 bytes CSI
            double csiTime = csiPacket->calculateTransmissionTime(20.0, 8, 5.0/6.0);
            csiPacket->setTransmissionTime(currentTime, currentTime + csiTime);
            transmittedPackets.push_back(std::move(csiPacket));
            user->setChannelState(true);
            currentTime += csiTime;
        }
        
        // Step 3: Parallel transmission for 15ms
        double parallelStart = currentTime;
        for ( auto user : wifi5Users) {
            if (user->canTransmit()) {
                auto dataPacket = user->createPacket();
                dataPacket->setTransmissionTime(parallelStart, parallelStart + PARALLEL_TIME);
                user->addTransmittedPacket(*dataPacket);
                transmittedPackets.push_back(std::move(dataPacket));
            }
        }
        currentTime += PARALLEL_TIME;
        
        // Reset channel state for next cycle
        for ( auto user : wifi5Users) {
            user->setChannelState(false);
        }
    }
}

double WiFi5AccessPoint::computeThroughput() {
    double totalBits = 0.0;
    for (const auto& packet : transmittedPackets) {
        totalBits += packet->getSize() * 8;
    }
    return totalBits / 1000000.0; // Mbps (assuming 1 second simulation)
}

std::pair<double, double> WiFi5AccessPoint::computeLatency() {
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
