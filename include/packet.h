#ifndef PACKET_H
#define PACKET_H

#include <string>

class Packet {
private:
    int size;           
    int sourceId;
    int destinationId;
    double transmissionStartTime;
    double transmissionEndTime;
    double latency;

public:
    Packet(int packetSize = 1024, int src = 0, int dest = 0);

    int getSize() const;
    int getSourceId() const;
    int getDestinationId() const;
    
    // Calculate transmission time based on channel parameters
    double calculateTransmissionTime(double bandwidth_mhz, int modulation_bits, double coding_rate) const;
    
    void setTransmissionTime(double start, double end);
    double getLatency() const;
    double getTransmissionStartTime() const;
    double getTransmissionEndTime() const;
};

#endif