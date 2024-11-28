#ifndef PACKET_H
#define PACKET_H


class Packet {
private:
    int size;           
    int sourceId;
    int destinationId;
    std::string data;

    void generateRandomData(int bitLength);

public:
    Packet(int packetSize = 1024, int src = 0, int dest = 0);

    int getSize() const;
    int getSourceId() const;
    std::string getData() const;

    int countOnes() const;
    int countZeros() const;
    double getOneProbability() const;
};



#endif // WIFI_SIMULATION_H