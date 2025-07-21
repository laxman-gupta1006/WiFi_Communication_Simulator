#include "../include/packet.h"
#include <algorithm>

Packet::Packet(int packetSize, int src, int dest) 
    : size(packetSize), sourceId(src), destinationId(dest), 
      transmissionStartTime(0.0), transmissionEndTime(0.0), latency(0.0) {
    // No data generation - just metadata for simulation
}

int Packet::getSize() const { return size; }
int Packet::getSourceId() const { return sourceId; }
int Packet::getDestinationId() const { return destinationId; }

// Calculate transmission time based on WiFi parameters
double Packet::calculateTransmissionTime(double bandwidth_mhz, int modulation_bits, double coding_rate) const {
    // Data rate = bandwidth * log2(modulation) * coding_rate
    double data_rate_mbps = bandwidth_mhz * modulation_bits * coding_rate; // Mbps
    double data_rate_bytes_per_ms = data_rate_mbps * 1000000.0 / 8.0 / 1000.0; // Bytes per ms
    return static_cast<double>(size) / data_rate_bytes_per_ms; // transmission time in ms
}

void Packet::setTransmissionTime(double start, double end) {
    transmissionStartTime = start;
    transmissionEndTime = end;
    latency = end - start;
}

double Packet::getLatency() const { return latency; }
double Packet::getTransmissionStartTime() const { return transmissionStartTime; }
double Packet::getTransmissionEndTime() const { return transmissionEndTime; }
