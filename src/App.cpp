#include "App.hpp"
#include "User.hpp"
#include "Vehicle.hpp"
#include "ChargingPort.hpp"
#include "ChargingSession.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace {

using namespace ftxui;

constexpr int kWidth = 76;  // lebar border terluar

// badge = teks dengan blok warna di belakangnya
Element badge(const std::string& s, Color bg) {
    return text(" " + s + " ") | bold | color(Color::Black) | bgcolor(bg);
}

// merah (rendah), kuning (menengah), hijau (hampir penuh)
Color batteryColor(double pct) {
    if (pct < 30.0) return Color::Red;
    if (pct < 70.0) return Color::Yellow;
    return Color::Green;
}

// 12345 -> "Rp 12.345"
std::string rupiah(double v) {
    std::string s = std::to_string(static_cast<long long>(v));
    for (int i = static_cast<int>(s.size()) - 3; i > 0; i -= 3) s.insert(i, ".");
    return "Rp " + s;
}

// margin kiri-kanan 1 spasi di dalam border terluar
Element pad(Element e) {
    return hbox({text(" "), std::move(e) | flex, text(" ")});
}

Element cell(Element e, int w) { return std::move(e) | size(WIDTH, EQUAL, w); }

// satu baris tabel port.
Element tableRow(Element c1, Element c2, Element c3,
                 Element c4, Element c5, Element c6) {
    return hbox({
        cell(std::move(c1), 6),
        cell(std::move(c2), 13),
        cell(std::move(c3), 14),
        cell(std::move(c4), 21),
        cell(std::move(c5), 5),
        std::move(c6) | flex,
    });
}

}  // namespace

void StationApp::run() {
    using namespace ftxui;

    // objek domain (dibuat saat runtime)
    User user("Fr", 100000.0);

    std::vector<std::unique_ptr<ChargingPort>> ports;
    ports.push_back(std::make_unique<DCFastPort>(1));
    ports.push_back(std::make_unique<ACStandardPort>(2));

    // satu slot per port. Null = port kosong.
    std::vector<std::unique_ptr<Vehicle>> vehicles(ports.size());
    std::vector<std::unique_ptr<ChargingSession>> sessions(ports.size());

    // state form
    std::string plate, pctStr = "20", message;
    std::vector<std::string> typeLabels = {"Car", "Motorcycle"};
    std::vector<std::string> portLabels = {"Port 1 (DC)", "Port 2 (AC)"};
    int typeIdx = 0, portIdx = 0;

    const double speedup = 60.0;  // 1 detik nyata = 1 menit simulasi
    const auto start = std::chrono::steady_clock::now();
    auto last = start;

    auto screen = ScreenInteractive::Fullscreen();

    // aksi
    auto startCharging = [&] {
        size_t p = static_cast<size_t>(portIdx);
        if (sessions[p]) { message = "Port sedang dipakai."; return; }
        if (plate.empty()) { message = "Plat nomor masih kosong."; return; }
        double pct;
        try { pct = std::stod(pctStr); }
        catch (...) { message = "Persen baterai tidak valid."; return; }
        pct = std::clamp(pct, 0.0, 100.0);

        // hanya titik ini yang tahu tipe konkret kendaraan
        std::unique_ptr<Vehicle> v;
        if (typeIdx == 0) v = std::make_unique<Car>(plate, 50.0, 50.0 * pct / 100.0);
        else              v = std::make_unique<Motorcycle>(plate, 4.0, 4.0 * pct / 100.0);

        ports[p]->plugVehicle(v.get());
        sessions[p] = std::make_unique<ChargingSession>(&user, ports[p].get());
        vehicles[p] = std::move(v);
        message = "Mulai mengisi di Port " + std::to_string(ports[p]->getID()) + ".";
        plate.clear();
    };

    auto stopCharging = [&](size_t p) {
        if (!sessions[p]) { message = "Port ini kosong."; return; }
        double cost = sessions[p]->getCurrentCost();
        bool paid = sessions[p]->stopSession();   // port dicabut di dalam
        sessions[p].reset();
        vehicles[p].reset();                      // bebaskan kendaraan setelah dicabut
        message = paid ? "Lunas: " + rupiah(cost)
                       : "Saldo tidak cukup untuk " + rupiah(cost);
    };

    // komponen input
    auto plateInput = Input(&plate, "AB 1234 CD");
    auto pctInput   = Input(&pctStr, "0-100");
    auto typeToggle = Toggle(&typeLabels, &typeIdx);
    auto portToggle = Toggle(&portLabels, &portIdx);
    auto btnStart = Button("Mulai",   startCharging);
    auto btnStop1 = Button("Stop P1", [&] { stopCharging(0); });
    auto btnStop2 = Button("Stop P2", [&] { stopCharging(1); });
    auto btnQuit  = Button("Keluar",  screen.ExitLoopClosure());

    auto controls = Container::Vertical({
        plateInput, pctInput, typeToggle, portToggle,
        Container::Horizontal({btnStart, btnStop1, btnStop2, btnQuit}),
    });

    // render
    auto ui = Renderer(controls, [&] {
        auto now = std::chrono::steady_clock::now();
        double dt = std::chrono::duration<double>(now - last).count();
        double elapsed = std::chrono::duration<double>(now - start).count();
        last = now;

        for (size_t i = 0; i < sessions.size(); ++i) {
            if (!sessions[i]) continue;
            sessions[i]->simulateTime(dt * speedup / 3600.0);
            if (!sessions[i]->isActive()) {   // berhenti otomatis karena saldo habis
                message = "Saldo habis. Port " + std::to_string(ports[i]->getID())
                + " dihentikan, dibayar " + rupiah(sessions[i]->getCurrentCost());
                sessions[i].reset();
                vehicles[i].reset();
            }
        }

        const bool ready = elapsed >= 2.5;

        // --- header (maskot + judul) ---
        bool plus = (static_cast<int>(elapsed) % 2 == 0);
        std::string face = plus ? " + ᴗ + " : " - ᴗ - ";
        auto mascot = text(face) | color(Color::Cyan) | borderRounded;

        std::string status = ready
            ? "Ready. Selamat datang, " + user.getName()
            : "Launching app" + std::string(1 + static_cast<int>(elapsed * 2) % 3, '.');

        auto title = vbox({
            hbox({badge("Ampowered", Color::Cyan), text(" EV Charging Station") | bold}),
            text(status) | dim,
        }) | vcenter;

        auto header = pad(hbox({mascot, text("  "), title}));

        if (!ready) {
            return vbox({header}) | borderRounded | size(WIDTH, EQUAL, kWidth);
        }

        // --- tabel port ---
        Elements portRows;
        portRows.push_back(tableRow(
            text("Port") | bold, text("Tipe") | bold, text("Kendaraan") | bold,
            text("Baterai") | bold, text("kW") | bold, text("Biaya") | bold));
        portRows.push_back(separatorLight());

        for (size_t i = 0; i < ports.size(); ++i) {
            const Vehicle* v = ports[i]->getVehicle();   // polymorphism lewat Vehicle*
            if (v && sessions[i]) {
                double pct = v->getBatteryPercentage();
                portRows.push_back(tableRow(
                    text(std::to_string(ports[i]->getID())),
                    text(ports[i]->getTypeName()),
                    text(v->getPlate()),
                    hbox({gauge(pct / 100.0) | color(batteryColor(pct)) | flex,
                          text(" " + std::to_string(static_cast<int>(pct)) + "%")
                              | size(WIDTH, EQUAL, 5)}),
                    text(std::to_string(static_cast<double>(v->calcChargeSpeed()))),
                    text(rupiah(sessions[i]->getCurrentCost()))));
            } else {
                portRows.push_back(tableRow(
                    text(std::to_string(ports[i]->getID())),
                    text(ports[i]->getTypeName()),
                    badge("KOSONG", Color::GrayLight),
                    text("-"), text("-"), text("-")));
            }
        }
        portRows.push_back(text(""));
        portRows.push_back(hbox({text("Saldo: "),
                                 badge(rupiah(user.getBalance()), Color::Green)}));
        auto portSection = pad(vbox(std::move(portRows)));

        // --- form isi kendaraan ---
        auto formSection = pad(vbox({
            text("Isi kendaraan") | bold,
            hbox({text("Plat    : "), plateInput->Render() | size(WIDTH, EQUAL, 16)}),
            hbox({text("Tipe    : "), typeToggle->Render()}),
            hbox({text("Baterai : "), pctInput->Render() | size(WIDTH, EQUAL, 6),
                  text(" %")}),
            hbox({text("Port    : "), portToggle->Render()}),
        }));

        // --- navigasi + pesan ---
        auto navSection = pad(vbox({
            hbox({btnStart->Render(), text(" "), btnStop1->Render(), text(" "),
                  btnStop2->Render(), text(" "), btnQuit->Render()}),
            text(message) | color(Color::Yellow),
        }));

        // border terluar
        return vbox({
            header,
            separator(),
            portSection,
            separator(),
            formSection,
            separator(),
            navSection,
        }) | borderRounded | size(WIDTH, EQUAL, kWidth);
    });

    // pemicu refresh ~10x per detik
    std::atomic<bool> running{true};
    std::thread ticker([&] {
        while (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            screen.PostEvent(Event::Custom);
        }
    });

    screen.Loop(ui);
    running = false;
    ticker.join();
}