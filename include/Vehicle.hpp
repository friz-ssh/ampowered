#pragma once
#include <memory>
#include <string>

class Vehicle {
private:
    std::string plate;
    double batteryCapacity;   // kWh, kapasitas maksimal
    double currentCharge;     // kWh, isi saat ini

public:
    Vehicle(std::string plate, double capacity, double percent);
    virtual ~Vehicle() = default;

    virtual double calcChargeSpeed() const = 0;

    double injectEnergy(double kwh);
    double getBatteryPercentage() const;
    const std::string& getPlate() const;
};

class Car : public Vehicle {
public:
    Car(std::string plate, double percent);
    double calcChargeSpeed() const override;
};

class Motorcycle : public Vehicle {
public:
    Motorcycle(std::string plate, double percent);
    double calcChargeSpeed() const override;
};

// memilih tipe konkret berdasarkan input
std::unique_ptr<Vehicle> makeVehicle(bool isCar, const std::string& plate, double percent);
