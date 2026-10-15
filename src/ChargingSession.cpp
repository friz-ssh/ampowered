#include "ChargingSession.hpp"

ChargingSession::ChargingSession(User* u, ChargingPort* p)
    : user(u), port(p), totalCost(0), active(true) {}

void ChargingSession::simulateTime(double hours) {
    if (!active) return;
    double kwh = port->processCharging(hours);
    totalCost += kwh * port->getRatePerKwh();

    // biaya tidak boleh melebihi saldo. sesi selesai sendiri saat saldo habis.
    if (totalCost >= user->getBalance()) {
        totalCost = user->getBalance();
        stopSession();
    }
}

// overload dipilih compiler dari tipe argumen: simulateTime(5) memanggil versi int
void ChargingSession::simulateTime(int minutes) {
    simulateTime(minutes / 60.0);
}

void ChargingSession::stopSession() {
    if (!active) return;
    user->deductBalance(totalCost);
    active = false;
    port->unplugVehicle();
}

double ChargingSession::getCurrentCost() const { return totalCost; }
bool ChargingSession::isActive() const { return active; }
