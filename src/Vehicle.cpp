#include "Vehicle.hpp"

Vehicle::Vehicle(std::string plate, double capacity, double percent)
    : plate(plate), batteryCapacity(capacity > 0 ? capacity : 1.0), currentCharge(0) {
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    currentCharge = batteryCapacity * percent / 100.0;
}

double Vehicle::injectEnergy(double kwh) {
    if (kwh < 0) return 0.0;
    double space = batteryCapacity - currentCharge;   // sisa ruang
    double accepted = kwh;
    if (accepted > space) accepted = space;
    currentCharge += accepted;
    return accepted;                                  // yang benar-benar masuk
}

double Vehicle::getBatteryPercentage() const {
    return (currentCharge / batteryCapacity) * 100.0;
}

const std::string& Vehicle::getPlate() const { return plate; }

// Car
Car::Car(std::string plate, double percent)
    : Vehicle(plate, 50.0, percent) {}

double Car::calcChargeSpeed() const {
    if (getBatteryPercentage() > 80.0) { return 10.0; }
    return 50.0;
}

// Motorcycle
Motorcycle::Motorcycle(std::string plate, double percent)
    : Vehicle(plate, 4.0, percent) {}

double Motorcycle::calcChargeSpeed() const {
    if (getBatteryPercentage() > 90.0) { return 0.5; }
    return 3.0;
}

// pembuat objek
std::unique_ptr<Vehicle> makeVehicle(bool isCar, const std::string& plate,
                                     double percent) {
    if (isCar) return std::make_unique<Car>(plate, percent);
    return std::make_unique<Motorcycle>(plate, percent);
}
