#pragma once
#include <string>
#include <vector>

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

namespace tui {

constexpr int kWidth = 76;   // lebar border terluar

// teks dengan blok warna
ftxui::Element badge(const std::string& s, ftxui::Color bg);

// badge selebar teksnya aja
ftxui::Element cellBadge(const std::string& s, ftxui::Color bg);

// merah (rendah), kuning (menengah), hijau (hampir penuh)
ftxui::Color batteryColor(double pct);

// 12345 -> "Rp 12.345"
std::string rupiah(double v);

// Angka dengan 1 desimal tetap: 50 -> "50.0"
std::string fixed1(double v);

// margin kiri-kanan 1 spasi di dalam border terluar
ftxui::Element pad(ftxui::Element e);

ftxui::Component lineInput(std::string* value, const std::string& placeholder,
                           std::function<void()> onEnter = [] {});

ftxui::ComponentDecorator numericFilter(std::string* value, size_t maxLen, bool allowDot);

ftxui::Element tableRow(ftxui::Element c1, ftxui::Element c2, ftxui::Element c3,
                        ftxui::Element c4, ftxui::Element c5, ftxui::Element c6);

ftxui::Component toggle(const std::vector<std::string>* entries, int* selected);

}  // namespace tui
