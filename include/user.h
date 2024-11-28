#ifndef USER_H
#define USER_H

class User {
protected:
    int id;
    std::mt19937 rng;

public:
    User(int userId);

    virtual std::unique_ptr<Packet> createPacket() = 0;
    virtual bool canTransmit() = 0;
    int getId() const;
    virtual ~User() = default;
};

#endif // WIFI_SIMULATION_H