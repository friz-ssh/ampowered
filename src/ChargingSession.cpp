#include "ChargingSession.hpp"

ChargingSession::ChargingSession(User* u, ChargingPort* p)
                :user(u), port(p), totalEnergy(0), totalCost(0), active(true) {}

void ChargingSession::simulateTime(double hours) {
    if (!active) return;
    double kwh = port->processCharging(hours);
    totalEnergy += kwh;
    totalCost += kwh * port->getRatePerKwh();

    // batasin biaya sebesar saldo, kemudian sesi langsung berhenti
    if (totalCost >= user->getBalance()) {
        totalCost = user->getBalance();
        stopSession();
    }
}

bool ChargingSession::stopSession() {
    if (!active) return false;
    if (!user->deductBalance(totalCost)) return false;   // gagal: sesi tetap aktif
    active = false;
    port->unplugVehicle();
    return true;
}

double ChargingSession::getTotalEnergy() const { return totalEnergy; }
double ChargingSession::getCurrentCost() const { return totalCost; }
bool ChargingSession::isActive() const { return active; }