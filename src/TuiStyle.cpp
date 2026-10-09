#include "TuiStyle.hpp"

#include <ftxui/component/event.hpp>

#include <cmath>
#include <cstdio>

namespace tui {

using namespace ftxui;

Element badge(const std::string& s, Color bg) {
    return text(" " + s + " ") | bold | color(Color::Black) | bgcolor(bg);
}

Element cellBadge(const std::string& s, Color bg) {
    return hbox({badge(s, bg), filler()});
}

Color batteryColor(double pct) {
    if (pct < 30.0) return Color::Red;
    if (pct < 70.0) return Color::Yellow;
    return Color::Green;
}

std::string rupiah(double v) {
    std::string s = std::to_string(static_cast<long long>(v));
    for (int i = static_cast<int>(s.size()) - 3; i > 0; i -= 3) s.insert(i, ".");
    return "Rp " + s;
}

std::string fixed1(double v) {
    char buf[10];
    std::snprintf(buf, sizeof(buf), "%.1f", v);
    return buf;
}

Element pad(Element e) {
    return hbox({text(" "), std::move(e) | flex, text(" ")});
}

Component lineInput(std::string* value, const std::string& placeholder,
                    std::function<void()> onEnter) {
    InputOption opt = InputOption::Default();
    opt.multiline = false;
    opt.on_enter = std::move(onEnter);
    return Input(value, placeholder, opt);
}

ComponentDecorator numericFilter(std::string* value, size_t maxLen, bool allowDot) {
    return CatchEvent([=](Event e) {
        if (!e.is_character()) return false;          // panah, backspace, tab: lanjut normal
        const std::string& ch = e.character();
        bool isDigit = ch.size() == 1 && ch[0] >= '0' && ch[0] <= '9';
        bool isDot = allowDot && ch == "." && value->find('.') == std::string::npos;
        if (!isDigit && !isDot) return true;          // karakter lain ditolak
        return value->size() >= maxLen;               // sudah penuh: tolak
    });
}

namespace {
Element cell(Element e, int w) {
    return hbox({std::move(e) | size(WIDTH, EQUAL, w - 1), text(" ")});
}
}  // namespace

Element tableRow(Element c1, Element c2, Element c3,
                 Element c4, Element c5, Element c6) {
    return hbox({
        cell(std::move(c1), 6),    // Port
        cell(std::move(c2), 13),   // Tipe
        cell(std::move(c3), 15),   // Kendaraan
        cell(std::move(c4), 20),   // Baterai
        cell(std::move(c5), 6),    // kW
        std::move(c6) | flex,      // Biaya
    });
}

Component toggle(const std::vector<std::string>* entries, int* selected) {
    MenuOption opt = MenuOption::Toggle();
    opt.entries_option.transform = [](const EntryState& s) {
        Element e = text(" " + s.label + " ");
        if (s.focused) e = e | inverted;
        if (s.active) e = e | bold;
        if (!s.focused && !s.active) e = e | dim;
        return e;
    };
    // text biasa (bukan separator()) agar tidak menyambung ke garis section di bawahnya
    opt.elements_infix = [] { return text(" │ "); };
    return Menu(entries, selected, opt);
}

}  // namespace tui
