#include "../include/wifi4.h"
#include <random>
#include <thread>
#include <chrono>

WiFi4User::WiFi4User(int userId) 
    : User(userId), backoffTime(0), totalTransmissionTime(0.0), 
      totalLatency(0.0), MAX_BACKOFF(31) {}

std::unique_ptr<Packet> WiFi4User::createPacket() {
    return std::make_unique<Packet>(1024, id, 0); // 1KB packet to AP
}

bool WiFi4User::canTransmit() {
    return true; // Always has data to transmit for simulation
}

int WiFi4User::getBackoffTime() const { return backoffTime; }

void WiFi4User::incrementBackoff() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, std::min(MAX_BACKOFF, (1 << std::min(backoffTime + 1, 10)) - 1));
    backoffTime += dist(gen);
}

void WiFi4User::resetBackoff() { backoffTime = 0; }

double WiFi4User::getTotalTransmissionTime() const { return totalTransmissionTime; }
double WiFi4User::getTotalLatency() const { return totalLatency; }

void WiFi4User::addTransmissionTime(double time) { totalTransmissionTime += time; }
void WiFi4User::addLatency(double lat) { totalLatency += lat; }

const std::vector<Packet>& WiFi4User::getTransmittedPackets() const { return transmittedPackets; }

void WiFi4User::addTransmittedPacket(const Packet& packet) {
    transmittedPackets.push_back(packet);
}

WiFi4AccessPoint::WiFi4AccessPoint(int apId) 
    : AccessPoint(apId), channelBusy(false), currentTime(0.0) {}

bool WiFi4AccessPoint::isChannelFree() {
    std::lock_guard<std::mutex> lock(channelMutex);
    return !channelBusy;
}

void WiFi4AccessPoint::occupyChannel(double duration) {
    std::lock_guard<std::mutex> lock(channelMutex);
    channelBusy = true;
    // In a real simulation, you'd set a timer to release after duration
    // For simplicity, we'll release immediately after transmission calculation
    std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>(duration * 1000)));
    channelBusy = false;
}

void WiFi4AccessPoint::simulateTransmission() {
    currentTime = 0.0;
    
    // Convert users to WiFi4Users
    std::vector<WiFi4User*> wifi4Users;
    for (auto& user : users) {
        if (auto wifi4User = dynamic_cast<WiFi4User*>(user.get())) {
            wifi4Users.push_back(wifi4User);
        }
    }
    
    while (currentTime < SIMULATION_TIME) {
        bool anyTransmission = false;
        
        for (auto user : wifi4Users) {
            if (user->canTransmit()) {
                // Check if channel is free
                if (isChannelFree()) {
                    // Create and transmit packet
                    auto packet = user->createPacket();
                    double txTime = packet->calculateTransmissionTime(20.0, 8, 5.0/6.0); // WiFi 4 params
                    
                    packet->setTransmissionTime(currentTime, currentTime + txTime);
                    user->addTransmittedPacket(*packet);
                    user->addTransmissionTime(txTime);
                    user->addLatency(txTime + user->getBackoffTime());
                    user->resetBackoff();
                    
                    transmittedPackets.push_back(std::move(packet));
                    occupyChannel(txTime);
                    
                    currentTime += txTime;
                    anyTransmission = true;
                } else {
                    // Channel busy - backoff
                    user->incrementBackoff();
                    currentTime += user->getBackoffTime();
                }
            }
        }
        
        if (!anyTransmission) {
            currentTime += 1.0; // Advance time if no transmissions
        }
    }
}

double WiFi4AccessPoint::computeThroughput() {
    double totalBits = 0.0;
    for (const auto& packet : transmittedPackets) {
        totalBits += packet->getSize() * 8; // Convert to bits
    }
    return totalBits / (SIMULATION_TIME * 1000.0); // Mbps
}

std::pair<double, double> WiFi4AccessPoint::computeLatency() {
    if (users.empty()) return {0.0, 0.0};
    
    double totalLatency = 0.0;
    double maxLatency = 0.0;
    int count = 0;
    
    for (auto& user : users) {
        if (auto wifi4User = dynamic_cast<WiFi4User*>(user.get())) {
            double userLatency = wifi4User->getTotalLatency();
            totalLatency += userLatency;
            maxLatency = std::max(maxLatency, userLatency);
            count++;
        }
    }
    
    return {count > 0 ? totalLatency / count : 0.0, maxLatency};
}
