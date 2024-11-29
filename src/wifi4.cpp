#include"../include/wifi4.h"
#include"../include/channel.h"
#include<thread>

WiFi4User::WiFi4User(int userId):User(userId),backoffTime(0),MAX_BACKOFF(10) {}

std::unique_ptr<Packet> WiFi4User::createPacket() {
    try {
        return std::make_unique<Packet>(1024,id,rand()%100);
    } catch(const std::exception& e) {
        throw std::runtime_error("Error creating packet: "+std::string(e.what()));
    }
}

bool WiFi4User::canTransmit() {
    try {
        return true;
    } catch(const std::exception& e) {
        throw std::runtime_error("Error checking transmit capability: "+std::string(e.what()));
    }
}

double WiFi4User::getBackoffTime() {
    try {
        return backoffTime;
    } catch(const std::exception& e) {
        throw std::runtime_error("Error getting backoff time: "+std::string(e.what()));
    }
}

void WiFi4User::setBackoffTime() {
    try {
        backoffTime+=1;
    } catch(const std::exception& e) {
        throw std::runtime_error("Error setting backoff time: "+std::string(e.what()));
    }
}

void WiFi4User::setTransmissionTime() {
    try {
        TransmissionTime+=(6140000);
    } catch(const std::exception& e) {
        throw std::runtime_error("Error setting transmission time: "+std::string(e.what()));
    }
}

double WiFi4User::getTransmissionTime() {
    try {
        return TransmissionTime;
    } catch(const std::exception& e) {
        throw std::runtime_error("Error getting transmission time: "+std::string(e.what()));
    }
}

WiFi4AccessPoint::WiFi4AccessPoint(int apId):AccessPoint(apId),channelBusy(false) {}

void WiFi4AccessPoint::simulateTransmission() {
    try {
        std::vector<std::thread> userThreads;
        Channel channel;

        for (auto &user : users) {
            userThreads.emplace_back([&,userPtr=user.get()]() {
                try {
                    WiFi4User *wifi4User=dynamic_cast<WiFi4User *>(userPtr);
                    if (wifi4User) {
                        double latency=0.0;
                        double congestionFactor=std::min(0.05*users.size(),0.5);

                        while (true) {
                            if (!channel.tryAcquire()) {
                                wifi4User->setBackoffTime();
                                std::this_thread::sleep_for(std::chrono::microseconds(1));
                                continue;
                            }
                            auto packet=wifi4User->createPacket();
                            {
                                std::lock_guard<std::mutex> lock(mutex);
                                wifi4User->setTransmissionTime();
                                transmittedPackets.push_back(std::move(packet));
                            }
                            channel.release();
                            break;
                        }
                    }
                } catch(const std::exception& e) {
                    throw std::runtime_error("Error simulating transmission: "+std::string(e.what()));
                }
            });
        }
        for (auto &thread : userThreads) {
            if (thread.joinable()) thread.join();
        }
    } catch(const std::exception& e) {
        throw std::runtime_error("Error in simulateTransmission: "+std::string(e.what()));
    }
}

double WiFi4AccessPoint::computeThroughput() {
    try {
        long double totalDataBits=0.0;
        long double totalTime=0.0;

        for (const auto& packet : transmittedPackets) {
            totalDataBits+=(packet->getSize()*8);
        }

        long double totalTransmissionTime=0.0;
        long double totalBackoffTime=0.0;
        for (auto& user : users) {
            WiFi4User* wifi4User=dynamic_cast<WiFi4User*>(user.get());
            if (wifi4User) {
                double transmissionTime=wifi4User->getTransmissionTime();
                totalTransmissionTime+=transmissionTime;
                double backoffTime=wifi4User->getBackoffTime();
                totalBackoffTime+=backoffTime;
            }
        }
        totalTime=totalTransmissionTime+totalBackoffTime;

        if (totalTime==0) {
            return 0.0;
        }
        totalTime*=1000;

        double throughputMbps=(totalDataBits/totalTime)/1000;
        return throughputMbps;
    } catch(const std::exception& e) {
        throw std::runtime_error("Error computing throughput: "+std::string(e.what()));
    }
}

std::pair<double,double> WiFi4AccessPoint::computeLatency() {
    try {
        std::lock_guard<std::mutex> lock(mutex);
        double totalTransmissionTime=0.0;
        double totalBackoffTime=0.0;
        double maxLat=0.0;

        for (auto& user : users) {
            WiFi4User* wifi4User=dynamic_cast<WiFi4User*>(user.get());
            if (wifi4User) {
                double transmissionTime=wifi4User->getTransmissionTime();
                totalTransmissionTime+=transmissionTime;
                double backoffTime=wifi4User->getBackoffTime();
                totalBackoffTime+=backoffTime;
                double totalLatency=backoffTime+transmissionTime;
                if (maxLat<totalLatency) {
                    maxLat=totalLatency;
                }
            }
        }
        double averageLatency=((totalBackoffTime+totalTransmissionTime)/users.size());
        return {averageLatency,maxLat};
    } catch(const std::exception& e) {
        throw std::runtime_error("Error computing latency: "+std::string(e.what()));
    }
}
