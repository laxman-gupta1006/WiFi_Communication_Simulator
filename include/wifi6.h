#ifndef WIFI_6_H
#define WIFI_6_H

#include "./ap.h"
#include "./channel.h"
#include "./packet.h"
#include "./user.h"
#include "./wifi5.h"



class WiFi6User : public WiFi5User {
public:
    WiFi6User(int userId);

    std::unique_ptr<Packet> createPacket() override;
    bool canTransmit() override;
};


class WiFi6AccessPoint : public AccessPoint {
private:
    const double CHANNEL_ALLOCATION_TIME;
    std::vector<int> subChannelSizes;

public:
    WiFi6AccessPoint(int apId);

    void simulateTransmission() override;
    double computeThroughput() override;
    std::pair<double, double> computeLatency() override;
};



#endif // WIFI_SIMULATION_H