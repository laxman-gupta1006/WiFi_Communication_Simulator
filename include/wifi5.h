#ifndef WIFI_5_H
#define WIFI_5_H

#include "./ap.h"
#include "./channel.h"
#include "./packet.h"
#include "./user.h"
#include "./wifi4.h"

class WiFi5User : public WiFi4User {
private:
    bool hasChannelState;

public:
    WiFi5User(int userId);  // Declare the constructor here
    std::unique_ptr<Packet> createPacket() override;
    bool canTransmit() override;
    void setChannelState(bool state);
};

class WiFi5AccessPoint : public AccessPoint {
private:
    const double PARALLEL_TIME;

public:
    WiFi5AccessPoint(int apId);

    void simulateTransmission() override;
    double computeThroughput() override;
    std::pair<double, double> computeLatency() override;
};


#endif // WIFI_SIMULATION_H