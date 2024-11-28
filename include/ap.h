#ifndef WIFI_SIMULATION_H
#define WIFI_SIMULATION_H

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


#endif // WIFI_SIMULATION_H