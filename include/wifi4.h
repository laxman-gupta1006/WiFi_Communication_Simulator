#ifndef WIFI_4_H
#define WIFI_4_H

#include <iostream>
#include <vector>
#include <memory>
#include <random>
#include <chrono>
#include <algorithm>
#include <mutex>
#include "./ap.h"
#include "./packet.h"
#include "./user.h"

class WiFi4User : public User {
private:
    int backoffTime;
    double totalTransmissionTime;
    double totalLatency;
    const int MAX_BACKOFF;
    std::vector<Packet> transmittedPackets;

public:
    WiFi4User(int userId);

    std::unique_ptr<Packet> createPacket() override;
    bool canTransmit() override;
    int getBackoffTime() const;
    void incrementBackoff();
    void resetBackoff();
    double getTotalTransmissionTime() const;
    double getTotalLatency() const;
    void addTransmissionTime(double time);
    void addLatency(double lat);
    const std::vector<Packet>& getTransmittedPackets() const;
    void addTransmittedPacket(const Packet& packet);
};

class WiFi4AccessPoint : public AccessPoint {
private:
    bool channelBusy;
    std::mutex channelMutex;
    double currentTime;
    const double SIMULATION_TIME = 1000.0; // 1 second in ms

public:
    WiFi4AccessPoint(int apId);

    void simulateTransmission() override;
    double computeThroughput() override;
    std::pair<double, double> computeLatency() override;
    bool isChannelFree();
    void occupyChannel(double duration);
};

#endif