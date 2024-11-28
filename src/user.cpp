#include "../include/user.h"
#include <chrono>

User::User(int userId) : id(userId), rng(std::chrono::steady_clock::now().time_since_epoch().count()) {}
int User::getId() const { return id; }
