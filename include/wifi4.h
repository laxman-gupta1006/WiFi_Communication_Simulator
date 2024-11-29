#ifndef WIFI_4_H
#define WIFI_4_H
#include <iostream>
#include <vector>
#include <memory>
#include <random>
#include <chrono>
#include <algorithm>
#include <iomanip>
#include <mutex> // Channel synchronization
#include "./ap.h"
// #include "./channel.h"
#include "./packet.h"
#include "./user.h"

class WiFi4User : public User {
private:
    double backoffTime=0;
    double TransmissionTime=0;
    const int MAX_BACKOFF;

public:
    WiFi4User(int userId);

    std::unique_ptr<Packet> createPacket() override;
    bool canTransmit() override;
    double getBackoffTime(); // Getter for backoff time
    void setBackoffTime();
    double getTransmissionTime();
    void setTransmissionTime();
};

class WiFi4AccessPoint : public AccessPoint {
private:
    bool channelBusy;
    std::mutex channelMutex; // Mutex for channel synchronization

public:
    WiFi4AccessPoint(int apId);

    void simulateTransmission() override;
    double computeThroughput() override;
    std::pair<double, double> computeLatency() override;
    void addUser(std::unique_ptr<WiFi4User> user) { users.push_back(std::move(user)); }  
};


#endif // WIFI_SIMULATION_H