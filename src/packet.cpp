#include "../include/packet.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <numeric>
#include <chrono>
#include <iomanip>
#include <string>
#include <random>
#include <algorithm>

Packet::Packet(int packetSize, int src, int dest) : size(packetSize), sourceId(src), destinationId(dest) {
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
