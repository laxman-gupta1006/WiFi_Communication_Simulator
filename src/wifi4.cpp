#include "../include/wifi4.h"
#include "../include/channel.h"
#include <thread>

WiFi4User::WiFi4User(int userId):User(userId),backoffTime(0),MAX_BACKOFF(10) {}

std::unique_ptr<Packet> WiFi4User::createPacket() {
    return std::make_unique<Packet>(1024,id,rand()%100);
}

bool WiFi4User::canTransmit() {
    return true;
}

double WiFi4User::getBackoffTime() { return backoffTime; }

void WiFi4User::setBackoffTime() { backoffTime+=1; }

void WiFi4User::setTransmissionTime() { TransmissionTime+=(6140000); }

double WiFi4User::getTransmissionTime() { return TransmissionTime; }

WiFi4AccessPoint::WiFi4AccessPoint(int apId):AccessPoint(apId),channelBusy(false) {}

void WiFi4AccessPoint::simulateTransmission() {
    std::vector<std::thread> userThreads;
    Channel channel;

    for (auto &user : users) {
        userThreads.emplace_back([&,userPtr=user.get()]() {
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
        });
    }
    for (auto &thread : userThreads) {
        if (thread.joinable()) thread.join();
    }
}

double WiFi4AccessPoint::computeThroughput() {
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
}

std::pair<double,double> WiFi4AccessPoint::computeLatency() {
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
}
