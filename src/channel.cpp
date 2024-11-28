#include "../include/channel.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <numeric>
#include <chrono>
#include <iomanip>
Channel::Channel() : busy(false) {}

bool Channel::isBusy() const {
    std::lock_guard<std::mutex> lock(mutex);
    return busy;
}

bool Channel::tryAcquire() {
    std::lock_guard<std::mutex> lock(mutex);
    if (!busy) {
        busy = true;
        return true;
    }
    return false;
}

void Channel::release() {
    std::lock_guard<std::mutex> lock(mutex);
    busy = false;
}

void Channel::waitUntilFree() {
    while (isBusy()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

