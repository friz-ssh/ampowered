#pragma once
#include "Vehicle.hpp"
#include <string>

class ChargingPort {
private:
    int portID;
    Vehicle* connectedVehicle;
    
public:
    explicit ChargingPort(int id); // explicit supaya compiler tidak langsung konversi otomatis
    virtual ~ChargingPort() = default;

    // virtual function yang bisa di override sama child class nya
    virtual double getRatePerKwh() const = 0;
    virtual std::string getTypeName() const = 0;

    bool plugVehicle(Vehicle* v); // false jika sudah terisi
    void unplugVehicle();
    double processCharging(double hours); // kWh yang benar benar masuk

    bool isAvailable() const;
    int getID() const;
    const Vehicle* getVehicle() const;
};

class DCFastPort : public ChargingPort {
public:
    explicit DCFastPort(int id);
    double getRatePerKwh() const override;
    std::string getTypeName() const override;
};

class ACStandardPort : public ChargingPort {
public:
    explicit ACStandardPort(int id);
    double getRatePerKwh() const override;
    std::string getTypeName() const override;
};