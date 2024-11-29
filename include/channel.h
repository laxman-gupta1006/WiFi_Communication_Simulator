#ifndef CHANNEL_H
#define CHANNEL_H
#include <mutex>

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

#endif // CHANNEL_H