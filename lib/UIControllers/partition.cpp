#include "partition.h"
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <string>

#if defined(_WIN32) || defined(_WIN64)
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
    #ifdef max
        #undef max
    #endif
    #ifdef min
        #undef min
    #endif
#elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    #include <sys/ioctl.h>
    #include <unistd.h>
#endif

namespace ui {

static int getConsoleWidth()
{
#if defined(_WIN32) || defined(_WIN64)
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return csbi.srWindow.Right - csbi.srWindow.Left + 1;
    }
#elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        return w.ws_col;
    }
#endif
    return 80;
}

static int getConsoleHeight()
{
#if defined(_WIN32) || defined(_WIN64)
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    }
#elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        return w.ws_row;
    }
#endif
    return 25;
}

Partition::Partition(Type type, float ratio1, float ratio2)
    : type_(type), r1_(ratio1), r2_(ratio2)
{
}

Partition::Region& Partition::full()       { return regions_[0]; }
Partition::Region& Partition::left()       { return type_ == TwoSideH || type_ == FourSide ? regions_[0] : regions_[0]; }
Partition::Region& Partition::right()      { return type_ == TwoSideH || type_ == FourSide ? regions_[1] : regions_[0]; }
Partition::Region& Partition::top()        { return type_ == TwoSideV ? regions_[0] : regions_[0]; }
Partition::Region& Partition::bottom()     { return type_ == TwoSideV ? regions_[1] : (type_ == FourSide ? regions_[2] : regions_[0]); }
Partition::Region& Partition::topLeft()    { return type_ == FourSide ? regions_[0] : regions_[0]; }
Partition::Region& Partition::topRight()   { return type_ == FourSide ? regions_[1] : regions_[0]; }
Partition::Region& Partition::bottomLeft() { return type_ == FourSide ? regions_[2] : regions_[0]; }
Partition::Region& Partition::bottomRight(){ return type_ == FourSide ? regions_[3] : regions_[0]; }

int Partition::alignmentBand(Align a) {
    switch (a) {
        case Align::Left: case Align::Center: case Align::Right:
        case Align::TopLeft: case Align::TopCenter: case Align::TopRight:
            return 0;
        case Align::MiddleLeft: case Align::MiddleCenter: case Align::MiddleRight:
            return 1;
        case Align::BottomLeft: case Align::BottomCenter: case Align::BottomRight:
            return 2;
    }
    return 0;
}

void Partition::calcRegionPos(Align align, int compW, int compH,
                              int regionW, int regionH,
                              int &outX, int &outY) const
{
    switch (align) {
        case Align::Left:
        case Align::TopLeft:        outX = 0;                outY = 0;                        break;
        case Align::Center:
        case Align::TopCenter:      outX = (regionW - compW) / 2;  outY = 0;                  break;
        case Align::Right:
        case Align::TopRight:       outX = regionW - compW;  outY = 0;                        break;
        case Align::MiddleLeft:     outX = 0;                outY = (regionH - compH) / 2;    break;
        case Align::MiddleCenter:   outX = (regionW - compW) / 2;  outY = (regionH - compH) / 2;  break;
        case Align::MiddleRight:    outX = regionW - compW;  outY = (regionH - compH) / 2;    break;
        case Align::BottomLeft:     outX = 0;                outY = regionH - compH;          break;
        case Align::BottomCenter:   outX = (regionW - compW) / 2;  outY = regionH - compH;    break;
        case Align::BottomRight:    outX = regionW - compW;  outY = regionH - compH;          break;
    }
}

void Partition::update(int yOffset)
{
    int cw = getConsoleWidth();
    int ch = getConsoleHeight();

    switch (type_) {
        case OneSide:
            regions_[0].x_ = 0;
            regions_[0].y_ = yOffset;
            regions_[0].w_ = cw;
            regions_[0].h_ = ch - yOffset;
            break;

        case TwoSideH: {
            int leftW = (int)(cw * r1_ / 100.0f);
            if (leftW < 1) leftW = 1;
            int rightW = cw - leftW;
            if (rightW < 1) { rightW = 1; leftW = cw - 1; }

            regions_[0].x_ = 0;
            regions_[0].y_ = yOffset;
            regions_[0].w_ = leftW;
            regions_[0].h_ = ch - yOffset;

            regions_[1].x_ = leftW;
            regions_[1].y_ = yOffset;
            regions_[1].w_ = rightW;
            regions_[1].h_ = ch - yOffset;
            break;
        }

        case TwoSideV: {
            int topH = (int)((ch - yOffset) * r1_ / 100.0f);
            if (topH < 1) topH = 1;
            int botH = (ch - yOffset) - topH;
            if (botH < 1) { botH = 1; topH = (ch - yOffset) - 1; }

            regions_[0].x_ = 0;
            regions_[0].y_ = yOffset;
            regions_[0].w_ = cw;
            regions_[0].h_ = topH;

            regions_[1].x_ = 0;
            regions_[1].y_ = yOffset + topH;
            regions_[1].w_ = cw;
            regions_[1].h_ = botH;
            break;
        }

        case FourSide: {
            int hSplit = (int)(cw * r1_ / 100.0f);
            int vSplit = (int)((ch - yOffset) * r2_ / 100.0f);
            if (hSplit < 1) hSplit = 1;
            if (vSplit < 1) vSplit = 1;

            int rw = cw - hSplit;   if (rw < 1) rw = 1;
            int bh = (ch - yOffset) - vSplit; if (bh < 1) bh = 1;

            regions_[0].x_ = 0;              regions_[0].y_ = yOffset;
            regions_[0].w_ = hSplit;         regions_[0].h_ = vSplit;

            regions_[1].x_ = hSplit;         regions_[1].y_ = yOffset;
            regions_[1].w_ = rw;             regions_[1].h_ = vSplit;

            regions_[2].x_ = 0;              regions_[2].y_ = yOffset + vSplit;
            regions_[2].w_ = hSplit;         regions_[2].h_ = bh;

            regions_[3].x_ = hSplit;         regions_[3].y_ = yOffset + vSplit;
            regions_[3].w_ = rw;             regions_[3].h_ = bh;
            break;
        }
    }
}

void Partition::render(std::ostream &out)
{
    int numRegions = 0;
    switch (type_) {
        case OneSide:  numRegions = 1; break;
        case TwoSideH: numRegions = 2; break;
        case TwoSideV: numRegions = 2; break;
        case FourSide: numRegions = 4; break;
    }

    for (int ri = 0; ri < numRegions; ++ri) {
        Region &reg = regions_[ri];
        if (reg.w_ <= 0 || reg.h_ <= 0) continue;

        // Tell all components in this region to use external positioning
        for (auto comp : reg.comps_) {
            if (comp) comp->setUsesExternalPositioning(true);
        }

        // Fill region background if set
        if (!reg.bgColor_.empty()) {
            int r = 0, g = 0, b = 0;
            if (reg.bgColor_.size() == 7 && reg.bgColor_[0] == '#') {
                try {
                    r = std::stoi(reg.bgColor_.substr(1,2), nullptr, 16);
                    g = std::stoi(reg.bgColor_.substr(3,2), nullptr, 16);
                    b = std::stoi(reg.bgColor_.substr(5,2), nullptr, 16);
                } catch (...) {}
            }
            std::string bgAnsi = "\x1b[48;2;" + std::to_string(r) + ';'
                               + std::to_string(g) + ';' + std::to_string(b) + 'm';
            for (int row = 0; row < reg.h_; ++row) {
                out << "\x1b[" << (reg.y_ + row + 1) << ";" << (reg.x_ + 1) << "H";
                out << bgAnsi;
                for (int c = 0; c < reg.w_; ++c) out << ' ';
                out << "\x1b[0m";
            }
        }

        for (auto comp : reg.comps_) {
            if (!comp) continue;

            int compW = comp->width();
            int compH = comp->height();
            if (compW < 6) compW = 6;

            int cx, cy;
            calcRegionPos(comp->alignment(), compW, compH,
                          reg.w_, reg.h_, cx, cy);

            // Apply displacement
            float dx = comp->displacementX();
            float dy = comp->displacementY();
            int dirX = 1;
            switch (comp->alignment()) {
                case Align::Right: case Align::TopRight:
                case Align::MiddleRight: case Align::BottomRight:
                    dirX = -1; break;
                default: break;
            }
            int dirY = 1;
            switch (comp->alignment()) {
                case Align::BottomLeft: case Align::BottomCenter:
                case Align::BottomRight:
                    dirY = -1; break;
                default: break;
            }
            cx += (int)(dx * compW * dirX / 100.0f);
            cy += (int)(dy * compH * dirY / 100.0f);

            // Offset by region position on screen
            cx += reg.x_;
            cy += reg.y_;

            if (cx < 0) cx = 0;
            if (cy < 0) cy = 0;

            // Render line by line
            std::string content = comp->toString();
            size_t pos = 0;
            int lineNum = 0;
            while (pos < content.size()) {
                size_t next = content.find('\n', pos);
                std::string line = (next == std::string::npos)
                    ? content.substr(pos)
                    : content.substr(pos, next - pos);
                out << "\x1b[" << (cy + lineNum + 1) << ";" << (cx + 1) << "H";
                out << line;
                ++lineNum;
                if (next == std::string::npos) break;
                pos = next + 1;
            }
        }
    }
}

void Partition::setFocus(bool on)
{
    hasFocus_ = on;
    if (on) focusFirstFocusable();
}

IComponent* Partition::focusedComponent() const
{
    if (!hasFocus_) return nullptr;
    int ri = focusRegion_;
    int ci = focusComp_;
    int numRegions = 0;
    switch (type_) {
        case OneSide:  numRegions = 1; break;
        case TwoSideH: numRegions = 2; break;
        case TwoSideV: numRegions = 2; break;
        case FourSide: numRegions = 4; break;
    }
    if (ri < 0 || ri >= numRegions) return nullptr;
    auto &comps = regions_[ri].comps_;
    if (ci < 0 || ci >= (int)comps.size()) return nullptr;
    return comps[ci];
}

void Partition::focusFirstFocusable()
{
    hasFocus_ = false;
    int numRegions = 0;
    switch (type_) {
        case OneSide:  numRegions = 1; break;
        case TwoSideH: numRegions = 2; break;
        case TwoSideV: numRegions = 2; break;
        case FourSide: numRegions = 4; break;
    }
    for (int ri = 0; ri < numRegions; ++ri) {
        auto &comps = regions_[ri].comps_;
        for (int ci = 0; ci < (int)comps.size(); ++ci) {
            if (comps[ci] && comps[ci]->isFocusable()) {
                focusRegion_ = ri;
                focusComp_ = ci;
                hasFocus_ = true;
                comps[ci]->setSelected(true);
                return;
            }
        }
    }
}

void Partition::moveFocus(int dRegion, int dComp)
{
    if (!hasFocus_) return;
    int numRegions = 0;
    switch (type_) {
        case OneSide:  numRegions = 1; break;
        case TwoSideH: numRegions = 2; break;
        case TwoSideV: numRegions = 2; break;
        case FourSide: numRegions = 4; break;
    }

    // Deselect current
    IComponent *cur = focusedComponent();
    if (cur) cur->setSelected(false);

    // Try moving within current region first
    if (dComp != 0) {
        auto &comps = regions_[focusRegion_].comps_;
        int n = (int)comps.size();
        int ci = focusComp_;
        for (int i = 1; i <= n; ++i) {
            int next = (ci + (dComp > 0 ? i : -i) + n) % n;
            if (comps[next] && comps[next]->isFocusable()) {
                focusComp_ = next;
                comps[next]->setSelected(true);
                return;
            }
        }
    }

    // Try adjacent region
    if (dRegion != 0) {
        int ri = focusRegion_ + dRegion;
        while (ri >= 0 && ri < numRegions) {
            auto &comps = regions_[ri].comps_;
            for (int ci = 0; ci < (int)comps.size(); ++ci) {
                if (comps[ci] && comps[ci]->isFocusable()) {
                    focusRegion_ = ri;
                    focusComp_ = ci;
                    comps[ci]->setSelected(true);
                    return;
                }
            }
            ri += dRegion;
        }
    }

    // Fallback: re-select current
    if (cur) cur->setSelected(true);
}

bool Partition::handleKey(int key)
{
    if (!hasFocus_) {
        if (key >= 32 && key <= 126) return false;
        return false;
    }

    IComponent *focused = focusedComponent();

    // Arrows: navigate within partition
    if (key == 75 || key == 'D') { // Left arrow
        moveFocus(0, -1);
        return true;
    }
    if (key == 77 || key == 'C') { // Right arrow
        moveFocus(0, 1);
        return true;
    }
    if (key == 72 || key == 'A') { // Up arrow
        moveFocus(-1, 0);
        return true;
    }
    if (key == 80 || key == 'B') { // Down arrow
        moveFocus(1, 0);
        return true;
    }

    // Enter: activate focused
    if (key == 13 || key == 10) {
        if (focused) focused->onActivate();
        return true;
    }

    // Escape: not handled (falls through to MapComponent)
    if (key == 27) {
        return false;
    }

    // Delegate to the focused component
    if (focused && focused->handleKey(key)) {
        return true;
    }

    return false;
}

} // namespace ui
