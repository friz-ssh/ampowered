#pragma once
#include <memory>
#include <vector>

#include "ChargingPort.hpp"
#include "User.hpp"

class StationApp {
private:
    User& user;
    std::vector<std::unique_ptr<ChargingPort>>& ports;

public:
    StationApp(User& user, std::vector<std::unique_ptr<ChargingPort>>& ports);
    void run();
};
