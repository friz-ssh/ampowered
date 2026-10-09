#include "Vehicle.hpp"
#include <algorithm>
#include <utility>

// Vehicle
Vehicle::Vehicle(std::string plate, double capacity, double current)
        : plate(std::move(plate)), batteryCapacity(capacity > 0 ? capacity : 1.0), currentCharge(0) {
        if (current < 0) current = 0;
        if (current > batteryCapacity) current = batteryCapacity;
        currentCharge = current;
}

double Vehicle::injectEnergy(double kwh) {
    if (kwh < 0 ) return 0.0;
    double space = batteryCapacity - currentCharge; // sisa ruang
    double accepted = kwh;
    if (accepted > space) accepted = space;
    currentCharge += accepted;
    return accepted;
}

double Vehicle::getBatteryPercentage() const {
    return (currentCharge / batteryCapacity) * 100.0;
}

double Vehicle::getBatteryCapacity() const { return batteryCapacity; }
double Vehicle::getCurrentCharge() const { return currentCharge; }
const std::string& Vehicle::getPlate() const { return plate; }

// Car
Car::Car(std::string plate, double capacity, double current)
    : Vehicle(std::move(plate), capacity, current) {}

double Car::calcChargeSpeed() const {
    if (getBatteryPercentage() > 80.0) { return 10.0; }
    else { return 50.0; }
}

// Motorcycle
Motorcycle::Motorcycle(std::string plate, double capacity, double current)
           :Vehicle(std::move(plate), capacity, current) {}

double Motorcycle::calcChargeSpeed() const {
    if (getBatteryPercentage() > 90.0) { return 0.5; }
    else { return 3.0; }
}

