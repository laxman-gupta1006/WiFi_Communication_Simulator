#include "../include/wifi4.h"
#include <thread>

WiFi4User::WiFi4User(int userId) : User(userId), backoffTime(0), MAX_BACKOFF(10) {}

std::unique_ptr<Packet> WiFi4User::createPacket() {
    return std::make_unique<Packet>(1024, id, rand() % 100); // Random destination for simulation
}

bool WiFi4User::canTransmit() {
    return true;
}

double WiFi4User::getBackoffTime(){return backoffTime;}

void WiFi4User::setBackoffTime(){backoffTime+=1;}

void WiFi4User::setTransmissionTime(){TransmissionTime+=(0.0614);}

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
