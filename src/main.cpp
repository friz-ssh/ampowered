#include <memory>
#include <vector>

#include "App.hpp"
#include "ChargingPort.hpp"
#include "User.hpp"

int main() {
    User user("Pengguna", 100000.0);

    std::vector<std::unique_ptr<ChargingPort>> ports;
    ports.push_back(std::make_unique<DCFastPort>(1));
    ports.push_back(std::make_unique<ACStandardPort>(2));

    StationApp app(user, ports);
    app.run();
}
