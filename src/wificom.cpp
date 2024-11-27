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

int User::getId() const { return id; }

// WiFi4User Implementation
WiFi4User::WiFi4User(int userId) : User(userId), backoffTime(0), MAX_BACKOFF(10) {}

std::unique_ptr<Packet> WiFi4User::createPacket() {
    return std::make_unique<Packet>(1024, id, rand() % 100); // Random destination for simulation
}

bool WiFi4User::canTransmit() {
    if (backoffTime > 0) {
        // cout <<"for "<<backoffTime<<endl; 
        backoffTime--;
        return false;
    }

    std::uniform_int_distribution<> dist(0, MAX_BACKOFF);
    backoffTime = dist(rng);
    return true;
}

// WiFi5User Implementation
WiFi5User::WiFi5User(int userId) : User(userId), hasChannelState(false) {}

std::unique_ptr<Packet> WiFi5User::createPacket() {
    return std::make_unique<Packet>(200, id, rand() % 100);
}

bool WiFi5User::canTransmit() {
    return hasChannelState;
}

void WiFi5User::setChannelState(bool state) {
    hasChannelState = state;
}

// WiFi6User Implementation
WiFi6User::WiFi6User(int userId) : User(userId) {}

std::unique_ptr<Packet> WiFi6User::createPacket() {
    return std::make_unique<Packet>(1024, id, rand() % 100);
}

bool WiFi6User::canTransmit() {
    return true;
}

// DataAnalyzer Implementation
void DataAnalyzer::analyzeBitDistribution(const std::vector<std::unique_ptr<Packet>>& packets) {
    if (packets.empty()) return;

    int totalBits = 0;
    int totalOnes = 0;
    int totalZeros = 0;

    for (const auto& packet : packets) {
        totalBits += packet->getData().length();
        totalOnes += packet->countOnes();
        totalZeros += packet->countZeros();
    }

    std::cout << "\n--- Data Transmission Analysis ---\n";
    std::cout << "Total Packets: " << packets.size() << "\n";
    std::cout << "Total Bits: " << totalBits << "\n";
    std::cout << "Ones: " << totalOnes << " (" 
              << std::fixed << std::setprecision(2) 
              << (totalOnes * 100.0 / totalBits) << "%)\n";
    std::cout << "Zeros: " << totalZeros << " (" 
              << (totalZeros * 100.0 / totalBits) << "%)\n";
}

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

// WiFi4AccessPoint Implementation
WiFi4AccessPoint::WiFi4AccessPoint(int apId) : AccessPoint(apId), channelBusy(false) {}
void WiFi4AccessPoint::simulateTransmission() {
    std::vector<std::thread> userThreads;

    for (auto& user : users) {
        userThreads.emplace_back([&, userPtr = user.get()]() {
            if (userPtr->canTransmit()) {
                auto startTime = std::chrono::high_resolution_clock::now(); // Start time
                std::lock_guard<std::mutex> lock(channelMutex);

                auto packet = userPtr->createPacket();
                {
                    std::lock_guard<std::mutex> transmitLock(mutex);
                    transmittedPackets.push_back(std::move(packet));
                }
                
                auto endTime = std::chrono::high_resolution_clock::now(); // End time
                std::chrono::duration<double> elapsed = endTime - startTime;
                
                // Add latency for this packet
                {
                    std::lock_guard<std::mutex> latencyLock(mutex);
                    latencies.push_back(elapsed.count());
                }
            }
        });
    }

    for (auto& thread : userThreads) {
        if (thread.joinable()) thread.join();
    }
}

std::pair<double, double> WiFi4AccessPoint::computeLatency() {
    std::lock_guard<std::mutex> lock(mutex);
    if (latencies.empty()) return {0.0, 0.0};
    
    double totalLatency = std::accumulate(latencies.begin(), latencies.end(), 0.0);
    double averageLatency = totalLatency / latencies.size();
    double maxLatency = *std::max_element(latencies.begin(), latencies.end());
    
    return {averageLatency, maxLatency};
}

// void WiFi4AccessPoint::simulateTransmission() {
//     std::vector<std::thread> userThreads;

//     for (auto& user : users) {
//         userThreads.emplace_back([&, userPtr = user.get()]() {
//             cout<<"User want to transmit "<<userPtr->getId()<<endl;
//             if (userPtr->canTransmit()) {
//                 std::lock_guard<std::mutex> lock(channelMutex);
//                 auto packet = userPtr->createPacket();
//                 {
//                     std::lock_guard<std::mutex> transmitLock(mutex);
//                     cout<<"User is tranmitting using 4  "<<userPtr->getId()<<endl;
//                     transmittedPackets.push_back(std::move(packet));
//                 }
//                 latencies.push_back(1.0); // Simulate latency
//             }else{
//                 cout<<"wating user to tranmitt  "<<userPtr->getId()<<endl;
//             }
//         });
//     }

//     for (auto& thread : userThreads) {
//         if (thread.joinable()) thread.join();
//     }
// }

double WiFi4AccessPoint::computeThroughput() {
    double totalDataBits = 0.0;
    for (const auto& packet : transmittedPackets) {
        totalDataBits += packet->getSize() * 8; // Size in bits
    }
    return totalDataBits / 100; // Throughput in bits/sec
}

// std::pair<double, double> WiFi4AccessPoint::computeLatency() {
//     if (latencies.empty()) return {0.0, 0.0};

//     double avgLatency = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();
//     double maxLatency = *std::max_element(latencies.begin(), latencies.end());

//     return {avgLatency, maxLatency};
// }

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
// void WiFi5AccessPoint::simulateTransmission() {
//     std::vector<std::thread> userThreads;

//     for (auto& user : users) {
//         userThreads.emplace_back([&, userPtr = user.get()]() {
//             auto packet = userPtr->createPacket();
//             {
//                 std::lock_guard<std::mutex> lock(mutex);
//                 // cout<<"User is tranmitting using 5  "<<userPtr->getId()<<endl;
//                 transmittedPackets.push_back(std::move(packet));
//             }
//             latencies.push_back(PARALLEL_TIME); // Simulated latency
//         });
//     }

//     for (auto& thread : userThreads) {
//         if (thread.joinable()) thread.join();
//     }
// }

double WiFi5AccessPoint::computeThroughput() {
    double totalDataBits = 0.0;
    for (const auto& packet : transmittedPackets) {
        totalDataBits += packet->getSize() * 8;
    }
    return totalDataBits / PARALLEL_TIME; // Throughput in bits/sec
}

// std::pair<double, double> WiFi5AccessPoint::computeLatency() {
//     if (latencies.empty()) return {0.0, 0.0};

//     double avgLatency = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();
//     double maxLatency = *std::max_element(latencies.begin(), latencies.end());

//     return {avgLatency, maxLatency};
// }


// WiFi6AccessPoint Implementation
WiFi6AccessPoint::WiFi6AccessPoint(int apId)
    : AccessPoint(apId), CHANNEL_ALLOCATION_TIME(5.0), subChannelSizes({2, 4, 10}) {}

// void WiFi6AccessPoint::simulateTransmission() {
//     std::vector<std::thread> userThreads;

//     for (auto& user : users) {
//         userThreads.emplace_back([&, userPtr = user.get()]() {
//             auto packet = userPtr->createPacket();
//             {
//                 std::lock_guard<std::mutex> lock(mutex);
//                 cout<<"User is tranmitting using 6  "<<userPtr->getId()<<endl;
//                 transmittedPackets.push_back(std::move(packet));
//             }
//             latencies.push_back(CHANNEL_ALLOCATION_TIME); // Simulated latency
//         });
//     }

//     for (auto& thread : userThreads) {
//         if (thread.joinable()) thread.join();
//     }
// }

double WiFi6AccessPoint::computeThroughput() {
    double totalDataBits = 0.0;
    for (const auto& packet : transmittedPackets) {
        totalDataBits += packet->getSize() * 8;
    }
    return totalDataBits / CHANNEL_ALLOCATION_TIME; // Throughput in bits/sec
}

// std::pair<double, double> WiFi6AccessPoint::computeLatency() {
//     if (latencies.empty()) return {0.0, 0.0};

//     double avgLatency = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();
//     double maxLatency = *std::max_element(latencies.begin(), latencies.end());

//     return {avgLatency, maxLatency};
// }

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
