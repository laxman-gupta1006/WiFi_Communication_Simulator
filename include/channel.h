#ifndef CHANNEL_H
#define CHANNEL_H

class Channel {
public:
    Channel();
    bool isBusy() const;
    bool tryAcquire();
    void release();
    void waitUntilFree();

private:
    mutable std::mutex mutex;
    bool busy;
};

#endif // WIFI_SIMULATION_H