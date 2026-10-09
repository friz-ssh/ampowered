#pragma once
#include <string>

class Vehicle {
protected:
    std::string plate;
    double batteryCapacity; // kapasitas maksimal baterai (kWh)
    double currentCharge; // isi daya saat ini (kWh)
    
public:
    Vehicle(std::string plate, double batteryCapacity, double currentCharge);
    virtual ~Vehicle() = default;

    virtual double calcChargeSpeed() const = 0;
    double injectEnergy(double kwh);
    double getBatteryPercentage() const;
    double getBatteryCapacity() const;
    double getCurrentCharge() const;
    const std::string& getPlate() const;
};

class Car : public Vehicle {
public:
    Car(std::string plate, double capacity, double current);
    double calcChargeSpeed() const override;
};

class Motorcycle : public Vehicle {
public:
    Motorcycle(std::string plate, double capacity, double current);
    double calcChargeSpeed() const override;
};