#include"../include/wifi5.h"
#include<numeric>
#include<thread>
#include<vector>
#include<iostream>
#include<chrono>
#include<mutex>
#include<algorithm>
#include"../include/channel.h"

constexpr double PARALLEL_TIME_MS=15.0;
constexpr double CSI_PACKET_SIZE_BYTES=200.0;

WiFi5User::WiFi5User(int userId):WiFi4User(userId),hasChannelState(false) {}

std::unique_ptr<Packet> WiFi5User::createPacket() {
    try {
        return std::make_unique<Packet>(200,id,rand()%100);
    } catch(const std::exception& e) {
        throw std::runtime_error("Error creating packet: "+std::string(e.what()));
    }
}

bool WiFi5User::canTransmit() {
    try {
        return hasChannelState;
    } catch(const std::exception& e) {
        throw std::runtime_error("Error checking transmit capability: "+std::string(e.what()));
    }
}

void WiFi5User::setChannelState(bool state) {
    try {
        hasChannelState=state;
    } catch(const std::exception& e) {
        throw std::runtime_error("Error setting channel state: "+std::string(e.what()));
    }
}

bool WiFi5User::isInBeamformedRange() {
    try {
        return true;
    } catch(const std::exception& e) {
        throw std::runtime_error("Error checking beamformed range: "+std::string(e.what()));
    }
}

std::unique_ptr<Packet> WiFi5User::createChannelStatePacket(int size) {
    try {
        return std::make_unique<Packet>(size);
    } catch(const std::exception& e) {
        throw std::runtime_error("Error creating channel state packet: "+std::string(e.what()));
    }
}

WiFi5AccessPoint::WiFi5AccessPoint(int apId):AccessPoint(apId),PARALLEL_TIME(PARALLEL_TIME_MS) {}

void WiFi5AccessPoint::simulateTransmission() {
    try {
        std::cout<<"Access Point broadcasting channel state packet...\n";
        std::vector<std::thread> userThreads;
        Channel channel;

        for(auto& user:users) {
            userThreads.emplace_back([&,userPtr=user.get()]() {
                try {
                    WiFi5User* wifi5User=dynamic_cast<WiFi5User*>(userPtr);
                    if(wifi5User) {
                        auto channelStatePacket=wifi5User->createChannelStatePacket(200);
                        {
                            std::lock_guard<std::mutex> lock(mutex);
                            wifi5User->setTransmissionTime();
                            transmittedPackets.push_back(std::move(channelStatePacket));
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

        userThreads.clear();

        for(auto& user:users) {
            userThreads.emplace_back([&,userPtr=user.get()]() {
                try {
                    WiFi5User* wifi5User=dynamic_cast<WiFi5User*>(userPtr);
                    if(wifi5User) {
                        double congestionFactor=std::min(0.05*users.size(),0.5);
                        while(true) {
                            if(!channel.tryAcquire()) {
                                wifi5User->setBackoffTime();
                                std::this_thread::sleep_for(std::chrono::microseconds(1));
                                continue;
                            }
                            auto packet=wifi5User->createPacket();
                            {
                                std::lock_guard<std::mutex> lock(mutex);
                                wifi5User->setTransmissionTime();
                                transmittedPackets.push_back(std::move(packet));
                            }
                            channel.release();
                            break;
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

double WiFi5AccessPoint::computeThroughput() {
    try {
        long double totalDataBits=0.0;
        long double totalTime=0.0;

        for(const auto& packet:transmittedPackets) {
            totalDataBits+=(packet->getSize()*8);
        }

        double broadcastTimeMs=200.0;
        double sequentialTimeMs=users.size()*15.0;
        double parallelWindowTimeMs=PARALLEL_TIME_MS;

        totalTime=broadcastTimeMs+sequentialTimeMs+parallelWindowTimeMs;
        totalTime/=1000.0;

        std::cout<<"Total data bits: "<<totalDataBits<<" bits\n";
        std::cout<<"Total time (in seconds): "<<totalTime<<" seconds\n";

        if(totalTime==0) {
            return 0.0;
        }

        double throughputMbps=(totalDataBits/totalTime)/1000000;
        return throughputMbps;
    } catch(const std::exception& e) {
        throw std::runtime_error("Error computing throughput: "+std::string(e.what()));
    }
}

std::pair<double,double> WiFi5AccessPoint::computeLatency() {
    try {
        std::lock_guard<std::mutex> lock(mutex);
        double totalTransmissionTime=0.0;
        double maxLatency=0.0;

        double broadcastTimeMs=200.0;
        double sequentialTimeMs=users.size()*15.0;
        double parallelWindowTimeMs=PARALLEL_TIME_MS;

        totalTransmissionTime+=(broadcastTimeMs+sequentialTimeMs+parallelWindowTimeMs)/1000.0;

        for(auto& user:users) {
            WiFi5User* wifi5User=dynamic_cast<WiFi5User*>(user.get());
            if(wifi5User) {
                double transmissionTime=wifi5User->getTransmissionTime();
                totalTransmissionTime+=transmissionTime;
                if(maxLatency<transmissionTime) {
                    maxLatency=transmissionTime;
                }
            }
        }

        double averageLatency=totalTransmissionTime/users.size();
        return {averageLatency,maxLatency};
    } catch(const std::exception& e) {
        throw std::runtime_error("Error computing latency: "+std::string(e.what()));
    }
}
