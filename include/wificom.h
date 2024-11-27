#ifndef WIFI_SIMULATION_H
#define WIFI_SIMULATION_H

#include <iostream>
#include <vector>
#include <memory>
#include <random>
#include <chrono>
#include <algorithm>
#include <iomanip>
#include <functional>
#include <numeric>
#include <string>
#include <mutex> // For mutex

// Packet Class
class Packet {
private:
    int size;           
    int sourceId;
    int destinationId;
    std::string data;

    void generateRandomData(int bitLength);

public:
    Packet(int packetSize = 1024, int src = 0, int dest = 0);

    int getSize() const;
    int getSourceId() const;
    std::string getData() const;

    int countOnes() const;
    int countZeros() const;
    double getOneProbability() const;
};

// User Base Class
class User {
protected:
    int id;
    std::mt19937 rng;

public:
    User(int userId);

    virtual std::unique_ptr<Packet> createPacket() = 0;
    virtual bool canTransmit() = 0;

    int getId() const;
    virtual ~User() = default;
};

// WiFi4User
class WiFi4User : public User {
private:
    int backoffTime;
    const int MAX_BACKOFF;

public:
    WiFi4User(int userId);

    std::unique_ptr<Packet> createPacket() override;
    bool canTransmit() override;
};

// WiFi5User
class WiFi5User : public User {
private:
    bool hasChannelState;

public:
    WiFi5User(int userId);

    std::unique_ptr<Packet> createPacket() override;
    bool canTransmit() override;
    void setChannelState(bool state);
};

// WiFi6User
class WiFi6User : public User {
public:
    WiFi6User(int userId);

    std::unique_ptr<Packet> createPacket() override;
    bool canTransmit() override;
};

// Data Analyzer
class DataAnalyzer {
public:
    static void analyzeBitDistribution(const std::vector<std::unique_ptr<Packet>>& packets);
};

// Access Point Base Class
class AccessPoint {
protected:
    int id;
    double bandwidth;
    std::vector<std::unique_ptr<User>> users;
    std::vector<std::unique_ptr<Packet>> transmittedPackets;
    std::vector<double> latencies;
    mutable std::mutex mutex;

public:
    AccessPoint(int apId, double bw = 20);

    virtual void addUser(std::unique_ptr<User> user);
    virtual void simulateTransmission() = 0;
    virtual double computeThroughput() = 0;
    virtual std::pair<double, double> computeLatency() = 0;

    const std::vector<std::unique_ptr<Packet>>& getTransmittedPackets() const;
    int getId() const;
    const std::vector<std::unique_ptr<User>>& getUsers() const;
    virtual ~AccessPoint() = default;
};

// WiFi4AccessPoint
class WiFi4AccessPoint : public AccessPoint {
private:
    bool channelBusy;
    std::mutex channelMutex; // Mutex for channel synchronization

public:
    WiFi4AccessPoint(int apId);

    void simulateTransmission() override;
    double computeThroughput() override;
    std::pair<double, double> computeLatency() override;
};

// WiFi5AccessPoint
class WiFi5AccessPoint : public AccessPoint {
private:
    const double PARALLEL_TIME;

public:
    WiFi5AccessPoint(int apId);

    void simulateTransmission() override;
    double computeThroughput() override;
    std::pair<double, double> computeLatency() override;
};

// WiFi6AccessPoint
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
