#include "App.hpp"
#include "ChargingSession.hpp"
#include "TuiStyle.hpp"
#include "Vehicle.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <atomic>
#include <chrono>
#include <string>
#include <thread>

namespace {

// kendaraan dan sesi yang sedang berjalan di satu port (null = port kosong)
struct Slot {
    std::unique_ptr<Vehicle> vehicle;
    std::unique_ptr<ChargingSession> session;
};

}  // namespace

StationApp::StationApp(User& user, std::vector<std::unique_ptr<ChargingPort>>& ports)
    : user(user), ports(ports) {}

void StationApp::run() {
    using namespace ftxui;

    std::vector<Slot> slots(ports.size());

    // state form
    std::string plate, pctStr = "20", topUpStr, message;
    std::vector<std::string> typeLabels = {"Car", "Motorcycle"};
    std::vector<std::string> portLabels;
    for (const auto& p : ports)
        portLabels.push_back("Port " + std::to_string(p->getID()) + " (" + p->getTypeName() + ")");
    int typeIdx = 0, portIdx = 0;

    const double speedup = 60.0;  // 1 detik real life = 1 menit simulasi
    const auto start = std::chrono::steady_clock::now();
    auto last = start;

    auto screen = ScreenInteractive::Fullscreen();

    // aksi
    auto startCharging = [&] {
        size_t p = static_cast<size_t>(portIdx);
        if (slots[p].session) { message = "Port sedang dipakai."; return; }
        if (plate.empty()) { message = "Plat nomor masih kosong."; return; }
        double pct;
        try { pct = std::stod(pctStr); }
        catch (...) { message = "Persen baterai tidak valid."; return; }

        // hanya makeVehicle() yang tahu tipe konkret kendaraan. di sini cukup Vehicle
        slots[p].vehicle = makeVehicle(typeIdx == 0, plate, pct);
        ports[p]->plugVehicle(slots[p].vehicle.get());
        slots[p].session = std::make_unique<ChargingSession>(&user, ports[p].get());
        message = "Mulai mengisi di Port " + std::to_string(ports[p]->getID()) + ".";
        plate.clear();
    };

    auto stopCharging = [&](size_t p) {
        if (!slots[p].session) { message = "Port ini kosong."; return; }
        slots[p].session->stopSession();
        message = "Lunas: " + tui::rupiah(slots[p].session->getCurrentCost());
        slots[p].session.reset();
        slots[p].vehicle.reset();     // bebaskan kendaraan setelah port dicabut
    };

    auto doTopUp = [&] {
        if (topUpStr.empty()) { message = "Isi nominal top up dulu."; return; }
        double amount = std::stod(topUpStr);   // filter hanya meloloskan digit
        if (amount <= 0) { message = "Nominal top up harus lebih dari 0."; return; }
        user.topUp(amount);
        message = "Top up " + tui::rupiah(amount) + " berhasil.";
        topUpStr.clear();
    };

    // komponen input
    auto plateInput = Input(&plate, "AB 1234 CD");
    auto pctInput   = tui::lineInput(&pctStr, "0-100") | tui::numericFilter(&pctStr, 6, true);
    auto topUpInput = tui::lineInput(&topUpStr, "50000", doTopUp)
                      |tui::numericFilter(&topUpStr, 7, false);
    auto typeToggle = tui::toggle(&typeLabels, &typeIdx);
    auto portToggle = tui::toggle(&portLabels, &portIdx);

    auto btnTopUp = Button("Top Up", doTopUp, ButtonOption::Ascii());
    auto btnStart = Button("Mulai", startCharging);
    auto btnQuit  = Button("Keluar", screen.ExitLoopClosure());
    std::vector<Component> stopButtons;
    for (size_t i = 0; i < ports.size(); ++i)
        stopButtons.push_back(Button("Stop P" + std::to_string(ports[i]->getID()),
                                     [&, i] { stopCharging(i); }));

    auto navRow = Container::Horizontal({btnStart});
    for (auto& b : stopButtons) navRow->Add(b);
    navRow->Add(btnQuit);

    auto controls = Container::Vertical({
        plateInput, typeToggle, pctInput, portToggle,
        Container::Horizontal({topUpInput, btnTopUp}),
        navRow,
    });

    // render
    auto ui = Renderer(controls, [&] {
        auto now = std::chrono::steady_clock::now();
        double dt = std::chrono::duration<double>(now - last).count();
        double elapsed = std::chrono::duration<double>(now - start).count();
        last = now;

        // waktu maju: logika hanya tahu "jam", chrono hanya ada di sini
        for (size_t i = 0; i < slots.size(); ++i) {
            if (!slots[i].session) continue;
            slots[i].session->simulateTime(dt * speedup / 3600.0);
            if (!slots[i].session->isActive()) {      // selesai sendiri: saldo habis
                message = "Saldo habis. Port " + std::to_string(ports[i]->getID())
                        + " dihentikan, dibayar "
                        + tui::rupiah(slots[i].session->getCurrentCost());
                slots[i].session.reset();
                slots[i].vehicle.reset();
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
            hbox({tui::badge("Ampowered", Color::Cyan), text(" EV Charging Station") | bold}),
            text(status) | dim,
        }) | vcenter;

        auto header = tui::pad(hbox({mascot, text("  "), title}));

        if (!ready) {
            return vbox({header}) | borderRounded | size(WIDTH, EQUAL, tui::kWidth);
        }

        // --- tabel port ---
        Elements portRows;
        portRows.push_back(tui::tableRow(
            text("Port") | bold, text("Tipe") | bold, text("Kendaraan") | bold,
            text("Baterai") | bold, text("kW") | bold, text("Biaya") | bold));
        portRows.push_back(separatorLight());

        for (size_t i = 0; i < ports.size(); ++i) {
            const Vehicle* v = ports[i]->getVehicle();   // lewat pointer induk
            if (v && slots[i].session) {
                double pct = v->getBatteryPercentage();
                portRows.push_back(tui::tableRow(
                    text(std::to_string(ports[i]->getID())),
                    text(ports[i]->getTypeName()),
                    text(v->getPlate()),
                    hbox({gauge(pct / 100.0) | color(tui::batteryColor(pct)) | flex,
                          text(" " + std::to_string(static_cast<int>(pct)) + "%")
                              | size(WIDTH, EQUAL, 5)}),
                    text(tui::fixed1(v->calcChargeSpeed())),
                    text(tui::rupiah(slots[i].session->getCurrentCost()))));
            } else {
                portRows.push_back(tui::tableRow(
                    text(std::to_string(ports[i]->getID())),
                    text(ports[i]->getTypeName()),
                    tui::cellBadge("KOSONG", Color::GrayLight),
                    text("-"), text("-"), text("-")));
            }
        }
        portRows.push_back(text(""));
        portRows.push_back(hbox({text("Saldo: "),
                                 tui::badge(tui::rupiah(user.getBalance()), Color::Green)}));
        auto portSection = tui::pad(vbox(std::move(portRows)));

        // --- Section 3: form isi kendaraan ---
        auto formSection = tui::pad(vbox({
            text("Isi kendaraan") | bold,
            hbox({text("Plat    : "), plateInput->Render() | size(WIDTH, EQUAL, 16)}),
            hbox({text("Tipe    : "), typeToggle->Render()}),
            hbox({text("Baterai : "), pctInput->Render() | size(WIDTH, EQUAL, 8),
                  text(" %")}),
            hbox({text("Port    : "), portToggle->Render()}),
            hbox({text("Top up  : "), topUpInput->Render() | size(WIDTH, EQUAL, 10),
                  text(" "), btnTopUp->Render()}),
        }));

        // --- Section 4: navigasi + pesan ---
        Elements nav;
        nav.push_back(btnStart->Render());
        for (auto& b : stopButtons) { nav.push_back(text(" ")); nav.push_back(b->Render()); }
        nav.push_back(text(" "));
        nav.push_back(btnQuit->Render());
        auto navSection = tui::pad(vbox({
            hbox(std::move(nav)),
            text(message) | color(Color::Yellow),
        }));

        // Satu border terluar; antar section hanya garis pemisah selebar border
        return vbox({
            header, separator(),
            portSection, separator(),
            formSection, separator(),
            navSection,
        }) | borderRounded | size(WIDTH, EQUAL, tui::kWidth);
    });

    // ===== Pemicu refresh ~10x per detik =====
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
