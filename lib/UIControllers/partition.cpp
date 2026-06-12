#include "partition.h"
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <climits>
#include <string>
#include "UIModals/objectRenderer.h"

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

        // Parse background color once
        int bgR = 0, bgG = 0, bgB = 0;
        bool hasBg = !reg.bgColor_.empty();
        std::string bgAnsi;
        if (hasBg) {
            if (reg.bgColor_.size() == 7 && reg.bgColor_[0] == '#') {
                try {
                    bgR = std::stoi(reg.bgColor_.substr(1,2), nullptr, 16);
                    bgG = std::stoi(reg.bgColor_.substr(3,2), nullptr, 16);
                    bgB = std::stoi(reg.bgColor_.substr(5,2), nullptr, 16);
                } catch (...) {}
            }
            bgAnsi = "\x1b[48;2;" + std::to_string(bgR) + ';'
                   + std::to_string(bgG) + ';' + std::to_string(bgB) + 'm';
        }

        // Fill region background if set (cached to avoid regenerating every frame)
        if (hasBg) {
            if (reg.w_ != reg.cachedW_ || reg.h_ != reg.cachedH_) {
                std::ostringstream bgBuf;
                for (int row = 0; row < reg.h_; ++row) {
                    bgBuf << "\x1b[" << (reg.y_ + row + 1) << ";" << (reg.x_ + 1) << "H";
                    bgBuf << bgAnsi;
                    for (int c = 0; c < reg.w_; ++c) bgBuf << ' ';
                    bgBuf << "\x1b[0m";
                }
                reg.bgCache_ = bgBuf.str();
                reg.cachedW_ = reg.w_;
                reg.cachedH_ = reg.h_;
            }
            out << reg.bgCache_;
        }

        // --- Group components by alignment band for vertical stacking ---
        struct LayoutItem {
            IComponent* comp;
            ObjectRenderer* objRend;
            int compW;
            int compH;
            int cx;
            int cy;
        };
        std::vector<LayoutItem> bands[3];

        for (auto comp : reg.comps_) {
            if (!comp) continue;

            int compW = comp->width();
            int compH = comp->height();
            if (compW < 6) compW = 6;

            ObjectRenderer *objRend = dynamic_cast<ObjectRenderer*>(comp);
            if ((compW > reg.w_ || compH > reg.h_) && objRend && objRend->resizable()) {
                objRend->fitToBounds(reg.w_, reg.h_);
                compW = objRend->width();
                compH = objRend->height();
                if (compW < 6) compW = 6;
                if (compH < 1) compH = 1;
            }

            int band = alignmentBand(comp->alignment());
            if (band < 0 || band > 2) band = 0;

            int cx, _cy;
            calcRegionPos(comp->alignment(), compW, compH,
                          reg.w_, reg.h_, cx, _cy);

            bands[band].push_back({comp, objRend, compW, compH, cx, 0});
        }

        // Calculate Y positions with stacking within each band
        const int GAP = 1;
        for (int bi = 0; bi < 3; ++bi) {
            if (bands[bi].empty()) continue;
            if (bi == 0) {
                int y = 0;
                for (auto &item : bands[bi]) {
                    item.cy = y;
                    y += item.compH + GAP;
                }
            } else if (bi == 1) {
                int totalH = 0;
                for (auto &item : bands[bi]) totalH += item.compH + GAP;
                if (!bands[bi].empty()) totalH -= GAP;
                int y = totalH > reg.h_ ? 0 : (reg.h_ - totalH) / 2;
                for (auto &item : bands[bi]) {
                    item.cy = y;
                    y += item.compH + GAP;
                }
            } else {
                int y = reg.h_;
                for (auto it = bands[bi].rbegin(); it != bands[bi].rend(); ++it) {
                    y -= it->compH;
                    it->cy = y;
                    y -= GAP;
                }
            }
        }

        // Render all components using calculated layout positions
        for (int bi = 0; bi < 3; ++bi) {
            for (auto &item : bands[bi]) {
                int cx = item.cx;
                int cy = item.cy;

                // Apply displacement
                float dx = item.comp->displacementX();
                float dy = item.comp->displacementY();
                int dirX = 1;
                switch (item.comp->alignment()) {
                    case Align::Right: case Align::TopRight:
                    case Align::MiddleRight: case Align::BottomRight:
                        dirX = -1; break;
                    default: break;
                }
                int dirY = 1;
                switch (item.comp->alignment()) {
                    case Align::BottomLeft: case Align::BottomCenter:
                    case Align::BottomRight:
                        dirY = -1; break;
                    default: break;
                }
                cx += (int)(dx * item.compW * dirX / 100.0f);
                cy += (int)(dy * item.compH * dirY / 100.0f);

                cx += reg.x_;
                cy += reg.y_;

                if (cx < 0) cx = 0;
                if (cy < 0) cy = 0;

                // Render line by line
                std::string content = item.comp->toString();
                size_t pos = 0;
                int lineNum = 0;
                while (pos < content.size()) {
                    size_t next = content.find('\n', pos);
                    std::string line = (next == std::string::npos)
                        ? content.substr(pos)
                        : content.substr(pos, next - pos);

                    out << "\x1b[" << (cy + lineNum + 1) << ";" << (cx + 1) << "H";

                    if (hasBg) {
                        out << bgAnsi;
                        std::string resetStr = "\x1b[0m";
                        std::string replacement = resetStr + bgAnsi;
                        size_t p = 0;
                        while ((p = line.find(resetStr, p)) != std::string::npos) {
                            line.replace(p, resetStr.length(), replacement);
                            p += replacement.length();
                        }
                    }
                    out << line;

                    ++lineNum;
                    if (next == std::string::npos) break;
                    pos = next + 1;
                }
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

void Partition::moveFocus(int dx, int dy)
{
    if (!hasFocus_) return;
    int numRegions = 0;
    switch (type_) {
        case OneSide:  numRegions = 1; break;
        case TwoSideH: numRegions = 2; break;
        case TwoSideV: numRegions = 2; break;
        case FourSide: numRegions = 4; break;
    }

    IComponent *cur = focusedComponent();
    if (cur) cur->setSelected(false);

    // Horizontal: try region switching first
    if (dx != 0) {
        int ri = focusRegion_ + dx;
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
            ri += dx;
        }
        // No adjacent region with focusable → fall through to within-region horizontal
    }

    // Build list of focusable components with stacking-aware Y positions
    struct Item { int idx, cx, cy; };
    std::vector<Item> items;
    {
        auto &comps = regions_[focusRegion_].comps_;
        struct BandItem { int idx; int cw; int ch; int cx; int band; };
        std::vector<BandItem> bandItems;

        for (int i = 0; i < (int)comps.size(); ++i) {
            if (!comps[i] || !comps[i]->isFocusable()) continue;
            int cw = comps[i]->width();
            int ch = comps[i]->height();
            if (cw < 6) cw = 6;
            int cx, cy_unused;
            calcRegionPos(comps[i]->alignment(), cw, ch,
                          regions_[focusRegion_].w_, regions_[focusRegion_].h_, cx, cy_unused);
            int band = alignmentBand(comps[i]->alignment());
            bandItems.push_back({i, cw, ch, cx, band});
        }

        // Calculate stacked Y positions per band
        const int GAP = 1;
        int bandY[3] = {0, 0, 0};
        int bandTotalH[3] = {0, 0, 0};
        for (auto &bi : bandItems) {
            if (bi.band == 0) {
                bandTotalH[0] += bi.ch + GAP;
            } else if (bi.band == 1) {
                bandTotalH[1] += bi.ch + GAP;
            } else {
                bandTotalH[2] += bi.ch + GAP;
            }
        }
        for (int b = 0; b < 3; ++b) {
            if (bandTotalH[b] > 0) bandTotalH[b] -= GAP;
        }
        if (!bandItems.empty()) {
            bandY[1] = bandTotalH[1] > regions_[focusRegion_].h_
                ? 0 : (regions_[focusRegion_].h_ - bandTotalH[1]) / 2;
            bandY[2] = regions_[focusRegion_].h_ - bandTotalH[2];
        }
        int nextY[3] = {0, bandY[1], bandY[2]};
        for (auto &bi : bandItems) {
            int cy = nextY[bi.band];
            nextY[bi.band] += bi.ch + GAP;
            items.push_back({bi.idx, bi.cx + bi.cw / 2, cy + bi.ch / 2});
        }
    }

    if (!items.empty()) {
        int curCx = 0, curCy = 0;
        for (auto &it : items) {
            if (it.idx == focusComp_) { curCx = it.cx; curCy = it.cy; break; }
        }

        int best = -1;
        long long bestDist = LLONG_MAX;

        for (auto &it : items) {
            if (it.idx == focusComp_) continue;
            int dxc = it.cx - curCx;
            int dyc = it.cy - curCy;

            bool ok = true;
            if (dx > 0) ok = ok && (dxc > 0);
            else if (dx < 0) ok = ok && (dxc < 0);
            if (dy > 0) ok = ok && (dyc > 0);
            else if (dy < 0) ok = ok && (dyc < 0);
            if (!ok) continue;

            long long d2 = (long long)dxc * dxc + (long long)dyc * dyc;
            if (d2 < bestDist) { bestDist = d2; best = it.idx; }
        }

        // No candidate in direction → wrap to opposite side
        if (best < 0) {
            if (dy != 0) {
                // Vertical wrap: find farthest in opposite vertical direction
                int wrapIdx = -1;
                int extreme = (dy > 0) ? INT_MAX : INT_MIN;
                for (auto &it : items) {
                    if (it.idx == focusComp_) continue;
                    if ((dy > 0 && it.cy < extreme) || (dy < 0 && it.cy > extreme)) {
                        extreme = it.cy; wrapIdx = it.idx;
                    }
                }
                best = wrapIdx;
            } else if (dx != 0) {
                // Horizontal wrap: find farthest in opposite horizontal direction
                int wrapIdx = -1;
                int extreme = (dx > 0) ? INT_MAX : INT_MIN;
                for (auto &it : items) {
                    if (it.idx == focusComp_) continue;
                    if ((dx > 0 && it.cx < extreme) || (dx < 0 && it.cx > extreme)) {
                        extreme = it.cx; wrapIdx = it.idx;
                    }
                }
                best = wrapIdx;
            }
        }

        if (best >= 0) {
            focusComp_ = best;
            regions_[focusRegion_].comps_[best]->setSelected(true);
            return;
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

    // Directional navigation:
    //   Up / Down      → move vertically  within region
    //   Left / Right   → try adjacent region first,
    //                    fall back to horizontal within-region
    if (key == UI_KEY_UP) { // Up
        moveFocus(0, -1);
        return true;
    }
    if (key == UI_KEY_DOWN) { // Down
        moveFocus(0, 1);
        return true;
    }
    if (key == UI_KEY_LEFT) { // Left
        moveFocus(-1, 0);
        return true;
    }
    if (key == UI_KEY_RIGHT) { // Right
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
