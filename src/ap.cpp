#include "../include/ap.h"
#include "../include/user.h"
#include "../include/packet.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <numeric>
#include <chrono>
#include <iomanip>
#include <vector>

AccessPoint::AccessPoint(int apId, double bw) : id(apId), bandwidth(bw) {}

void AccessPoint::addUser(std::unique_ptr<User> user) {
    users.push_back(std::move(user));
}

const std::vector<std::unique_ptr<Packet>>& AccessPoint::getTransmittedPackets() const {
    return transmittedPackets;
}

int AccessPoint::getId() const { return id; }

int AccessPoint::getBandwidth() const { return bandwidth; }

const std::vector<std::unique_ptr<User>>& AccessPoint::getUsers() const {
    return users;
}
