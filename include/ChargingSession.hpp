#pragma once
#include "User.hpp"
#include "ChargingPort.hpp"

class ChargingSession {
private:
    User* user;
    ChargingPort* port;
    double totalEnergy;
    double totalCost;
    bool active;
    
public:
    ChargingSession(User* u, ChargingPort* p);

    void simulateTime(double hours); // maju sekian jam
    bool stopSession(); // false jika saldo sudah berkurang

    double getTotalEnergy() const;
    double getCurrentCost() const;
    bool isActive() const;
};