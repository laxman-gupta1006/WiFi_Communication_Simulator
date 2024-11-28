#include "../include/wificom.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <numeric>
#include <chrono>
#include <iomanip>
using namespace std;
// Packet Class Implementation
constexpr double PARALLEL_TIME_MS = 5.0; // Simulated parallel transmission delay in milliseconds

Packet::Packet(int packetSize, int src, int dest) 
    : size(packetSize), sourceId(src), destinationId(dest) {
    generateRandomData(size * 8);
}

void Packet::generateRandomData(int bitLength) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 1);

    data.clear();
    for (int i = 0; i < bitLength; ++i) {
        data += std::to_string(dis(gen));
    }
}

int Packet::getSize() const { return size; }
int Packet::getSourceId() const { return sourceId; }
std::string Packet::getData() const { return data; }
int Packet::countOnes() const { return std::count(data.begin(), data.end(), '1'); }
int Packet::countZeros() const { return std::count(data.begin(), data.end(), '0'); }
double Packet::getOneProbability() const { return static_cast<double>(countOnes()) / data.length(); }

// User Base Class Implementation
User::User(int userId) : id(userId), rng(std::chrono::steady_clock::now().time_since_epoch().count()) {}


Channel::Channel() : busy(false) {}

bool Channel::isBusy() const {
    std::lock_guard<std::mutex> lock(mutex);
    return busy;
}

bool Channel::tryAcquire() {
    std::lock_guard<std::mutex> lock(mutex);
    if (!busy) {
        busy = true;
        return true;
    }
    return false;
}

void Channel::release() {
    std::lock_guard<std::mutex> lock(mutex);
    busy = false;
}

void Channel::waitUntilFree() {
    while (isBusy()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}


int User::getId() const { return id; }

// AccessPoint Base Class Implementation
AccessPoint::AccessPoint(int apId, double bw) : id(apId), bandwidth(bw) {}

void AccessPoint::addUser(std::unique_ptr<User> user) {
    users.push_back(std::move(user));
}

const std::vector<std::unique_ptr<Packet>>& AccessPoint::getTransmittedPackets() const {
    return transmittedPackets;
}

int AccessPoint::getId() const { return id; }

const std::vector<std::unique_ptr<User>>& AccessPoint::getUsers() const {
    return users;
}


// WiFi4User Implementation
WiFi4User::WiFi4User(int userId) : User(userId), backoffTime(0), MAX_BACKOFF(10) {}

std::unique_ptr<Packet> WiFi4User::createPacket() {
    return std::make_unique<Packet>(1024, id, rand() % 100); // Random destination for simulation
}

bool WiFi4User::canTransmit() {
    return true;
}

double WiFi4User::getBackoffTime(){return backoffTime;}

void WiFi4User::setBackoffTime(){backoffTime+=1;}

void WiFi4User::setTransmissionTime(){TransmissionTime+=0.0614;}

double WiFi4User::getTransmissionTime(){return TransmissionTime;}

// WiFi4AccessPoint Implementation
WiFi4AccessPoint::WiFi4AccessPoint(int apId) : AccessPoint(apId), channelBusy(false) {}

void WiFi4AccessPoint::simulateTransmission() {
    std::vector<std::thread> userThreads;

    for (auto& user : users) {
        userThreads.emplace_back([&, userPtr = user.get()]() {
            WiFi4User* wifi4User = dynamic_cast<WiFi4User*>(userPtr);
            if (wifi4User) {
                while (true) {
                    // Wait if the channel is busy
                    if (channelBusy) {

                        std::this_thread::sleep_for(std::chrono::microseconds(1));
                        wifi4User->setBackoffTime();  // Increase backoff time after each retry
                        continue;  // Retry after backoff period
                    }

                    // If the channel is free, attempt transmission
                    {
                        std::lock_guard<std::mutex> lock(channelMutex);
                        channelBusy = true;  // Channel is now busy
                    }


                    // Create and transmit packet
                    auto packet = wifi4User->createPacket();
                    {
                        std::lock_guard<std::mutex> lock(mutex);
                        wifi4User->setTransmissionTime();
                        transmittedPackets.push_back(std::move(packet));  // Store the transmitted packet
                    }
        

                    // Store the latency for this transmission only onc

                    {
                        std::lock_guard<std::mutex> lock(channelMutex);
                        channelBusy = false;  // Channel is now free
                    }

                    break;  // Exit the loop after successful transmission
                }
            }
        });
    }

    // Wait for all user threads to finish
    for (auto& thread : userThreads) {
        if (thread.joinable()) thread.join();
    }
}

// WiFi4AccessPoint Throughput Calculation
double WiFi4AccessPoint::computeThroughput() {
    long double totalDataBits = 0.0;
    long double totalTime = 0.0;

    // Calculate total data transmitted (in bits)
    for (const auto& packet : transmittedPackets) {
        totalDataBits += (packet->getSize() * 8);  // Size in bits
    }

    // Calculate total time considering backoff times for each user
    long double totalTransmissionTime = 0.0;
    long double totalBackoffTime = 0.0;
    for (auto& user : users) {
        WiFi4User* wifi4User = dynamic_cast<WiFi4User*>(user.get());
        if (wifi4User) {
            double transmissionTime = wifi4User->getTransmissionTime();
            totalTransmissionTime += transmissionTime;

            double backoffTime = wifi4User->getBackoffTime();
            totalBackoffTime += backoffTime;
        }
    }

    totalTime = totalTransmissionTime + totalBackoffTime;

    std::cout << "Total data bits: " << totalDataBits << " bits\n";
    std::cout << "Total time (in seconds): " << totalTime << " seconds\n";

    if (totalTime == 0) {
        return 0.0;  // Avoid division by zero if totalTime is zero
    }

    // Convert total time to milliseconds
    totalTime *= 1000;  // Now totalTime is in milliseconds

    // Calculate throughput in Mbps
    double throughputMbps = (totalDataBits / totalTime) / 1000;  // Convert from bps to Mbps
    return throughputMbps;
}

// WiFi4AccessPoint Latency Calculation
std::pair<double, double> WiFi4AccessPoint::computeLatency() {
    std::lock_guard<std::mutex> lock(mutex);
    double totalTransmissionTime = 0.0;
    double totalBackoffTime = 0.0;
    double maxLat = 0.0;

    for (auto& user : users) {
        WiFi4User* wifi4User = dynamic_cast<WiFi4User*>(user.get());
        if (wifi4User) {
            double transmissionTime = wifi4User->getTransmissionTime();
            totalTransmissionTime += transmissionTime;

            double backoffTime = wifi4User->getBackoffTime();
            totalBackoffTime += backoffTime;

            double totalLatency = backoffTime + transmissionTime;
            if (maxLat < totalLatency) {
                maxLat = totalLatency;
            }
        }
    }

    double averageLatency = ((totalBackoffTime + totalTransmissionTime) / users.size());
    return {averageLatency, maxLat};
}


// WiFi5User Implementation
WiFi5User::WiFi5User(int userId) : WiFi4User(userId), hasChannelState(false) {}

std::unique_ptr<Packet> WiFi5User::createPacket() {
    return std::make_unique<Packet>(200, id, rand() % 100);
}

bool WiFi5User::canTransmit() {
    return hasChannelState;
}

 void WiFi5User::setChannelState(bool state) {
        hasChannelState = state;
    }

// WiFi5AccessPoint Implementation
WiFi5AccessPoint::WiFi5AccessPoint(int apId) : AccessPoint(apId), PARALLEL_TIME(15.0) {}


void WiFi5AccessPoint::simulateTransmission() {
    std::vector<std::thread> userThreads;

    for (auto& user : users) {
        userThreads.emplace_back([&, userPtr = user.get()]() {
            if (userPtr->canTransmit()) {
                auto startTime = std::chrono::high_resolution_clock::now(); // Start time
                
                // Simulate parallel transmissions with a realistic delay
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    auto packet = userPtr->createPacket();
                    transmittedPackets.push_back(std::move(packet));
                }

                auto endTime = std::chrono::high_resolution_clock::now(); // End time
                std::chrono::duration<double, std::milli> elapsed = endTime - startTime;

                // Simulate a constant parallel transmission time (e.g., 5ms for WiFi5)
                double computedLatency = elapsed.count() + PARALLEL_TIME_MS;
                {
                    std::lock_guard<std::mutex> latencyLock(mutex);
                    latencies.push_back(computedLatency);
                }
            }
        });
    }

    for (auto& thread : userThreads) {
        if (thread.joinable()) thread.join();
    }
}

std::pair<double, double> WiFi5AccessPoint::computeLatency() {
    std::lock_guard<std::mutex> lock(mutex);
    if (latencies.empty()) return {0.0, 0.0};

    double totalLatency = std::accumulate(latencies.begin(), latencies.end(), 0.0);
    double averageLatency = totalLatency / latencies.size();
    double maxLatency = *std::max_element(latencies.begin(), latencies.end());

    return {averageLatency, maxLatency};
}

double WiFi5AccessPoint::computeThroughput() {
    double totalDataBits = 0.0;
    for (const auto& packet : transmittedPackets) {
        totalDataBits += packet->getSize() * 8;
    }
    return totalDataBits / PARALLEL_TIME; // Throughput in bits/sec
}

// WiFi6User Implementation
WiFi6User::WiFi6User(int userId) : User(userId) {}

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
