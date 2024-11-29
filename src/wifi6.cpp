#include"../include/wifi6.h"
#include<numeric>
#include<thread>
#include<vector>
#include<numeric>
#include<chrono>
#include<iostream>

WiFi6User::WiFi6User(int userId):WiFi5User(userId) {}

std::unique_ptr<Packet> WiFi6User::createPacket() {
    try {
        return std::make_unique<Packet>(1024,id,rand()%100);
    } catch(const std::exception& e) {
        throw std::runtime_error("Error creating packet: "+std::string(e.what()));
    }
}

bool WiFi6User::canTransmit() {
    try {
        return true;
    } catch(const std::exception& e) {
        throw std::runtime_error("Error checking transmit capability: "+std::string(e.what()));
    }
}

WiFi6AccessPoint::WiFi6AccessPoint(int apId):AccessPoint(apId),CHANNEL_ALLOCATION_TIME(5.0),subChannelSizes({2,4,10}) {}

double WiFi6AccessPoint::computeThroughput() {
    try {
        double totalDataBits=0.0;
        for(const auto& packet:transmittedPackets) {
            totalDataBits+=packet->getSize()*8;
        }
        return totalDataBits/CHANNEL_ALLOCATION_TIME;
    } catch(const std::exception& e) {
        throw std::runtime_error("Error computing throughput: "+std::string(e.what()));
    }
}

void WiFi6AccessPoint::simulateTransmission() {
    try {
        std::vector<std::thread> userThreads;
        for(auto& user:users) {
            userThreads.emplace_back([&,userPtr=user.get()]() {
                try {
                    if(userPtr->canTransmit()) {
                        auto startTime=std::chrono::high_resolution_clock::now();
                        int subChannelSize=subChannelSizes[userPtr->getId()%subChannelSizes.size()];
                        {
                            std::lock_guard<std::mutex> lock(mutex);
                            auto packet=userPtr->createPacket();
                            transmittedPackets.push_back(std::move(packet));
                        }
                        auto endTime=std::chrono::high_resolution_clock::now();
                        std::chrono::duration<double> elapsed=endTime-startTime;
                        double adjustedLatency=elapsed.count()+CHANNEL_ALLOCATION_TIME/subChannelSize;
                        {
                            std::lock_guard<std::mutex> latencyLock(mutex);
                            latencies.push_back(adjustedLatency);
                        }
                    }
                } catch(const std::exception& e) {
                    throw std::runtime_error("Error simulating transmission for user: "+std::string(e.what()));
                }
            });
        }
        for(auto& thread:userThreads) {
            if(thread.joinable()) thread.join();
        }
    } catch(const std::exception& e) {
        throw std::runtime_error("Error in simulateTransmission: "+std::string(e.what()));
    }
}

std::pair<double,double> WiFi6AccessPoint::computeLatency() {
    try {
        std::lock_guard<std::mutex> lock(mutex);
        if(latencies.empty()) return {0.0,0.0};
        double totalLatency=std::accumulate(latencies.begin(),latencies.end(),0.0);
        double averageLatency=totalLatency/latencies.size();
        double maxLatency=*std::max_element(latencies.begin(),latencies.end());
        return {averageLatency,maxLatency};
    } catch(const std::exception& e) {
        throw std::runtime_error("Error computing latency: "+std::string(e.what()));
    }
}
