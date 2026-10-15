#include "ChargingPort.hpp"
#include <string>

ChargingPort::ChargingPort(int id): portID(id), connectedVehicle(nullptr) {}

bool ChargingPort::plugVehicle(Vehicle* v) {
    if (v == nullptr || connectedVehicle != nullptr) { return false; }
    connectedVehicle = v;
    return true;
}

void ChargingPort::unplugVehicle() { connectedVehicle = nullptr; }

double ChargingPort::processCharging(double hours) {
    if (connectedVehicle == nullptr || hours <= 0) { return 0.0; }
    double kw = connectedVehicle->calcChargeSpeed();
    return connectedVehicle->injectEnergy(kw * hours);
}

int ChargingPort::getID() const { return portID; }
const Vehicle* ChargingPort::getVehicle() const { return connectedVehicle; }

// DCFastPort
DCFastPort::DCFastPort(int id) : ChargingPort(id) {}
double DCFastPort::getRatePerKwh() const { return 3500.0; }
std::string DCFastPort::getTypeName() const { return "DC Fast"; }

// ACStandardPort
ACStandardPort::ACStandardPort(int id) : ChargingPort(id) {}
double ACStandardPort::getRatePerKwh() const { return 2000.0; }
std::string ACStandardPort::getTypeName() const { return "AC Standard"; }
