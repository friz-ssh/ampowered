#pragma once
#include "User.hpp"
#include "ChargingPort.hpp"

class ChargingSession {
private:
    User* user;
    ChargingPort* port;
    double totalCost;
    bool active;

public:
    ChargingSession(User* u, ChargingPort* p);

    void simulateTime(double hours); // maju sekian jam
    void simulateTime(int minutes); // overload: versi menit
    void stopSession(); // false jika saldo sudah berkurang

    double getCurrentCost() const;
    bool isActive() const;
};