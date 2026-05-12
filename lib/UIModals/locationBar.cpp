#include "locationBar.h"
#include <algorithm>
#include <iostream>
#include <sstream>

namespace ui {

LocationBar::LocationBar(const std::string &bgColor,
                         const std::string &fgColor,
                         int segment1, const std::string &text1, Align location1,
                         int segment2, const std::string &text2, Align location2,
                         int segment3, const std::string &text3, Align location3,
                         int width)
    : ConsoleInstance(), bgColor_(bgColor), fgColor_(fgColor), segments_(), width_(width)
{
    segments_.reserve(3);
    segments_.emplace_back(segment1, text1, location1);
    segments_.emplace_back(segment2, text2, location2);
    segments_.emplace_back(segment3, text3, location3);
}

static void placeText(std::string &buf, const std::string &txt, int pos)
{
    if (txt.empty()) return;
    if (pos < 0) pos = 0;
    int maxCopy = std::max(0, (int)buf.size() - pos);
    int copyLen = std::min((int)txt.size(), maxCopy);
    for (int i = 0; i < copyLen; ++i) buf[pos + i] = txt[i];
}

std::string LocationBar::toString() const
{
    if (width_ <= 0) return std::string();
    std::string buf(width_, ' ');

    for (const auto &seg : segments_) {
        if (seg.visible == HIDE) continue;
        int len = (int)seg.text.size();
        int pos = 0;
        switch (seg.align) {
            case Align::Left:
                pos = 0;
                break;
            case Align::Center:
                pos = (width_ - len) / 2;
                break;
            case Align::Right:
                pos = width_ - len;
                break;
        }
        if (pos < 0) pos = 0;
        placeText(buf, seg.text, pos);
    }

    return buf;
}

static bool parseHexColor(const std::string &hex, int &r, int &g, int &b)
{
    if (hex.size() != 7 || hex[0] != '#') return false;
    try {
        r = std::stoi(hex.substr(1,2), nullptr, 16);
        g = std::stoi(hex.substr(3,2), nullptr, 16);
        b = std::stoi(hex.substr(5,2), nullptr, 16);
        return true;
    } catch (...) {
        return false;
    }
}

void LocationBar::render(std::ostream &out)
{
#if defined(_WIN32) || defined(_WIN64)
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        if (GetConsoleMode(hOut, &mode)) {
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, mode);
        }
    }
#endif

    int consW = width_;
    int consH = 0;
    getConsoleSize(consW, consH);

    // Temporarily override width_ when building the line
    int oldW = width_;
    if (consW > 0) width_ = consW;
    std::string line = toString();
    width_ = oldW;

    int br=0, bg=0, bb=0;
    int fr=0, fg=0, fb=0;
    bool hasBg = parseHexColor(bgColor_, br, bg, bb);
    bool hasFg = parseHexColor(fgColor_, fr, fg, fb);

    std::ostringstream ss;
    if (hasBg) ss << "\x1b[48;2;" << br << ';' << bg << ';' << bb << 'm';
    if (hasFg) ss << "\x1b[38;2;" << fr << ';' << fg << ';' << fb << 'm';
    ss << line;
    ss << "\x1b[0m" << std::endl;

    out << ss.str();
}

} // namespace ui
