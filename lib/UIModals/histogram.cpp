#include "histogram.h"
#include <sstream>
#include <algorithm>
#include <cmath>

namespace ui {

Histogram::Histogram(
    const std::string &title,
    int width,
    int height,
    const std::string &bgColor,
    const std::string &fgColor,
    const std::string &borderColor,
    const std::string &titleBgColor,
    const std::string &titleFgColor)
    : ComponentBase(width, height)
    , title_(title)
    , bgColor_(bgColor)
    , fgColor_(fgColor)
    , borderColor_(borderColor)
    , titleBgColor_(titleBgColor)
    , titleFgColor_(titleFgColor)
{
    align_ = Align::TopLeft;
}

void Histogram::addBar(const std::string &label, float value, const std::string &color)
{
    bars_.push_back(HistogramBar(label, value, color));
    setDirty();
}

float Histogram::computeMax() const
{
    if (maxValue_ > 0) return maxValue_;
    float m = 0;
    for (const auto &b : bars_) {
        if (b.value > m) m = b.value;
    }
    return m > 0 ? m : 1;
}

std::string Histogram::renderTitle(int w, int bR, int bG, int bB,
                                   int tR, int tG, int tB,
                                   int tFgR, int tFgG, int tFgB) const
{
    std::ostringstream ss;
    auto reset = [&]() -> std::string { return render_.reset(); };
    ss << render_.fg(bR, bG, bB) << "\xe2\x94\x8c" << reset();
    ss << render_.bg(tR, tG, tB) << render_.fg(tFgR, tFgG, tFgB);
    int titleLen = (int)title_.size();
    int titlePad = w - 2 - titleLen;
    if (titlePad < 0) titlePad = 0;
    int leftPad = titlePad / 2;
    int rightPad = titlePad - leftPad;
    for (int i = 0; i < leftPad; ++i) ss << ' ';
    ss << title_;
    for (int i = 0; i < rightPad; ++i) ss << ' ';
    ss << reset() << render_.fg(bR, bG, bB) << "\xe2\x94\x90" << reset() << '\n';
    return ss.str();
}

std::string Histogram::renderBottom(int w, int bR, int bG, int bB) const
{
    std::ostringstream ss;
    ss << render_.fg(bR, bG, bB) << "\xe2\x94\x94";
    for (int i = 0; i < w - 2; ++i) ss << "\xe2\x94\x80";
    ss << "\xe2\x94\x98" << render_.reset() << '\n';
    return ss.str();
}

std::string Histogram::renderEmptyRow(int w, int bgR, int bgG, int bgB,
                                       int fgR, int fgG, int fgB,
                                       int bR, int bG, int bB) const
{
    std::ostringstream ss;
    auto reset = [&]() -> std::string { return render_.reset(); };
    ss << render_.fg(bR, bG, bB) << "\xe2\x94\x82" << reset();
    ss << render_.bg(bgR, bgG, bgB) << render_.fg(fgR, fgG, fgB);
    for (int i = 0; i < w - 2; ++i) ss << ' ';
    ss << reset();
    ss << render_.fg(bR, bG, bB) << "\xe2\x94\x82" << reset() << '\n';
    return ss.str();
}

std::string Histogram::renderBarMode(int w, int bgR, int bgG, int bgB,
                                      int fgR, int fgG, int fgB,
                                      int bR, int bG, int bB) const
{
    std::ostringstream ss;
    auto reset = [&]() -> std::string { return render_.reset(); };
    auto borderFg = [&]() -> std::string { return render_.fg(bR, bG, bB); };

    float maxVal = computeMax();
    int barArea = w - 4;

    int labelMaxLen = 0;
    for (const auto &b : bars_) {
        if ((int)b.label.size() > labelMaxLen)
            labelMaxLen = (int)b.label.size();
    }
    if (labelMaxLen > barArea / 3) labelMaxLen = barArea / 3;
    int valWidth = showValues_ ? 6 : 0;
    int availBar = barArea - labelMaxLen - valWidth - 1;
    if (availBar < 2) availBar = 2;

    int contentW = labelMaxLen + 1 + availBar + 5;

    for (const auto &bar : bars_) {
        int barR = 0, barG = 0, barB = 0;
        render_.parseHex(bar.color, barR, barG, barB);

        ss << borderFg() << "\xe2\x94\x82" << reset();
        ss << render_.bg(bgR, bgG, bgB) << render_.fg(fgR, fgG, fgB);

        std::string label = bar.label;
        if ((int)label.size() > labelMaxLen)
            label = label.substr(0, labelMaxLen);
        int labelPadL = 0;
        if ((int)label.size() < labelMaxLen)
            labelPadL = labelMaxLen - (int)label.size();
        ss << label;
        for (int i = 0; i < labelPadL; ++i) ss << ' ';
        ss << ' ';

        int fill = maxVal > 0 ? (int)std::round((bar.value / maxVal) * availBar) : 0;
        if (fill < 0) fill = 0;
        if (fill > availBar) fill = availBar;

        ss << reset();
        ss << render_.bg(barR, barG, barB) << render_.fg(barR, barG, barB);
        for (int i = 0; i < fill; ++i) ss << ' ';
        ss << reset();
        ss << render_.bg(bgR, bgG, bgB) << render_.fg(fgR, fgG, fgB);
        for (int i = fill; i < availBar; ++i) ss << ' ';
        ss << reset();

        if (showValues_) {
            ss << render_.bg(bgR, bgG, bgB) << render_.fg(fgR, fgG, fgB);
            std::string valStr = std::to_string((int)bar.value);
            int vPad = 5 - (int)valStr.size();
            for (int i = 0; i < vPad; ++i) ss << ' ';
            ss << valStr;
            ss << reset();
        }

        // Pad to full width to prevent ghosting
        int curContent = (int)labelMaxLen + 1 + availBar + (showValues_ ? 5 : 0);
        int rest = (w - 2) - curContent;
        if (rest > 0) {
            ss << render_.bg(bgR, bgG, bgB);
            for (int i = 0; i < rest; ++i) ss << ' ';
            ss << reset();
        }

        ss << borderFg() << "\xe2\x94\x82" << reset() << '\n';
    }

    int usedRows = 1 + (int)bars_.size();
    for (int r = usedRows; r < height_; ++r) {
        ss << renderEmptyRow(w, bgR, bgG, bgB, fgR, fgG, fgB, bR, bG, bB);
    }

    return ss.str();
}

std::string Histogram::renderPieMode(int w, int bgR, int bgG, int bgB,
                                      int fgR, int fgG, int fgB,
                                      int bR, int bG, int bB) const
{
    std::ostringstream ss;
    auto reset = [&]() -> std::string { return render_.reset(); };
    auto borderFg = [&]() -> std::string { return render_.fg(bR, bG, bB); };

    float total = 0;
    for (const auto &b : bars_) total += b.value;
    if (total <= 0) total = 1;

    struct Slice { float start; float end; std::string color; std::string label; float value; };
    std::vector<Slice> slices;
    float cumul = 0;
    for (const auto &b : bars_) {
        float angle = (b.value / total) * 2.0f * 3.14159265f;
        slices.push_back({cumul, cumul + angle, b.color, b.label, b.value});
        cumul += angle;
    }

    int drawH = height_ - 1;
    if (drawH < 3) drawH = 3;
    int drawW = w - 2;
    if (drawW < 6) drawW = 6;

    int maxR = (std::min(drawW, drawH * 2) - 2) / 2;
    if (maxR < 2) maxR = 2;
    int cx = drawW / 2;
    int cy = drawH / 2;

    // Pre-compute label position on each slice (at mid-angle, 40% radius)
    struct LabelPos { int col; int row; int id; };
    std::vector<LabelPos> labelPositions;
    for (int si = 0; si < (int)slices.size(); ++si) {
        float mid = (slices[si].start + slices[si].end) / 2.0f;
        float r = maxR * 0.4f;
        int lc = cx + (int)(r * std::cos(mid));
        int lr = cy + (int)(r * std::sin(mid) / 2.0f);
        if (lc >= 0 && lc < drawW && lr >= 0 && lr < drawH)
            labelPositions.push_back({lc, lr, si});
    }

    // Legend start row
    int legendStartRow = cy + maxR / 2 + 1;
    if (legendStartRow < 0) legendStartRow = 0;

    for (int row = 0; row < drawH; ++row) {
        ss << borderFg() << "\xe2\x94\x82" << reset();

        for (int col = 0; col < drawW; ++col) {
            // Check if this cell is a label position
            bool isLabel = false;
            int labelSlice = -1;
            for (auto &lp : labelPositions) {
                if (lp.col == col && lp.row == row) {
                    isLabel = true;
                    labelSlice = lp.id;
                    break;
                }
            }

            int dx = col - cx;
            int dy = row - cy;
            float dist = std::sqrt((float)(dx * dx) + (float)(dy * dy * 4));
            float angle = std::atan2((float)dy, (float)dx);
            if (angle < 0) angle += 2.0f * 3.14159265f;

            if (dist <= maxR) {
                int sliceIdx = -1;
                for (int si = 0; si < (int)slices.size(); ++si) {
                    if (angle >= slices[si].start && angle < slices[si].end) {
                        sliceIdx = si;
                        break;
                    }
                }
                if (sliceIdx < 0 && !slices.empty()) {
                    if (angle >= slices.back().end - 0.001f || angle < slices[0].start)
                        sliceIdx = (int)slices.size() - 1;
                }

                if (sliceIdx >= 0 && sliceIdx < (int)slices.size()) {
                    int sr = 0, sg = 0, sb = 0;
                    render_.parseHex(slices[sliceIdx].color, sr, sg, sb);
                    if (isLabel && labelSlice == sliceIdx) {
                        std::string num = std::to_string(sliceIdx + 1);
                        int fgL = 255 - sr > 128 ? 255 : 0;
                        ss << render_.bg(sr, sg, sb) << render_.fg(fgL, fgL, fgL) << num[0];
                    } else {
                        ss << render_.bg(sr, sg, sb) << render_.fg(sr, sg, sb) << ' ';
                    }
                } else {
                    ss << render_.bg(bgR, bgG, bgB) << ' ';
                }
                ss << reset();
            } else {
                ss << render_.bg(bgR, bgG, bgB) << ' ';
                ss << reset();
            }
        }

        // Draw legend on same row
        int legendIdx = row - legendStartRow;
        if (legendIdx >= 0 && legendIdx < (int)slices.size()) {
            int lr = 0, lg = 0, lb = 0;
            render_.parseHex(slices[legendIdx].color, lr, lg, lb);
            ss << render_.bg(bgR, bgG, bgB) << render_.fg(fgR, fgG, fgB) << ' ';
            ss << render_.bg(lr, lg, lb) << ' ' << reset();
            ss << render_.bg(bgR, bgG, bgB) << render_.fg(fgR, fgG, fgB);
            std::string legend = "#" + std::to_string(legendIdx + 1) + " "
                + slices[legendIdx].label + " " + std::to_string((int)slices[legendIdx].value);
            int used = drawW + 3 + (int)legend.size();
            int rest = (w - 2) - used;
            if (rest < 0) {
                int maxL = (w - 2) - drawW - 4;
                if (maxL < 0) maxL = 0;
                if ((int)legend.size() > maxL)
                    legend = legend.substr(0, maxL > 1 ? maxL - 1 : 0) + "\xe2\x80\xa6";
                rest = 0;
            }
            ss << legend;
            for (int i = 0; i < rest; ++i) ss << ' ';
            ss << reset();
        } else {
            ss << render_.bg(bgR, bgG, bgB);
            int rest = (w - 2) - drawW;
            for (int i = 0; i < rest; ++i) ss << ' ';
            ss << reset();
        }

        ss << borderFg() << "\xe2\x94\x82" << reset() << '\n';
    }

    for (int r = 1 + drawH; r < height_; ++r) {
        ss << renderEmptyRow(w, bgR, bgG, bgB, fgR, fgG, fgB, bR, bG, bB);
    }

    return ss.str();
}

std::string Histogram::renderGridMode(int w, int bgR, int bgG, int bgB,
                                       int fgR, int fgG, int fgB,
                                       int bR, int bG, int bB) const
{
    std::ostringstream ss;
    auto reset = [&]() -> std::string { return render_.reset(); };
    auto borderFg = [&]() -> std::string { return render_.fg(bR, bG, bB); };

    int cellW = 2;
    int maxCellsPerRow = (w - 4) / (cellW + 1);
    if (maxCellsPerRow < 1) maxCellsPerRow = 1;

    // Build grid rows and track total visual columns used per row
    struct GridLine {
        std::string before;  // content to put after border before cells
        std::vector<GridCell>* cells;
        int usedCells;
    };
    std::vector<GridLine> gridLines;
    for (const auto &gridRow : gridRows_) {
        if (!gridRow.label.empty()) {
            gridLines.push_back({gridRow.label, nullptr, 0});
        }
        if (!gridRow.cells.empty()) {
            gridLines.push_back({"", const_cast<std::vector<GridCell>*>(&gridRow.cells),
                                 std::min((int)gridRow.cells.size(), maxCellsPerRow)});
        }
    }

    int rows = 0;
    for (auto &gl : gridLines) {
        if (1 + rows >= height_) break;

        ss << borderFg() << "\xe2\x94\x82" << reset();
        ss << render_.bg(bgR, bgG, bgB);

        if (!gl.before.empty()) {
            ss << render_.fg(fgR, fgG, fgB) << " " << gl.before;
            int used = 1 + (int)gl.before.size();
            int pad = (w - 2) - used;
            if (pad < 0) pad = 0;
            for (int i = 0; i < pad; ++i) ss << ' ';
            ss << reset();
        } else if (gl.cells) {
            ss << " ";
            int col = 0;
            for (size_t ci = 0; ci < gl.cells->size() && col < maxCellsPerRow; ++ci) {
                int cr = 0, cg = 0, cb = 0;
                render_.parseHex((*gl.cells)[ci].color, cr, cg, cb);
                ss << reset();
                ss << render_.bg(cr, cg, cb) << render_.fg(cr, cg, cb);
                for (int i = 0; i < cellW; ++i) ss << ' ';
                ss << reset();
                ss << render_.bg(bgR, bgG, bgB) << " ";
                col++;
            }
            int used = 1 + col * (cellW + 1);
            int pad = (w - 2) - used;
            if (pad < 0) pad = 0;
            for (int i = 0; i < pad; ++i) ss << ' ';
            ss << reset();
        } else {
            for (int i = 0; i < w - 2; ++i) ss << ' ';
            ss << reset();
        }

        ss << borderFg() << "\xe2\x94\x82" << reset() << '\n';
        rows++;
    }

    for (int r = 1 + rows; r < height_; ++r) {
        ss << renderEmptyRow(w, bgR, bgG, bgB, fgR, fgG, fgB, bR, bG, bB);
    }

    return ss.str();
}

std::string Histogram::toString() const
{
    int w = width_;
    if (w < 20) w = 20;

    int tR = 0, tG = 0, tB = 0;
    int tFgR = 0, tFgG = 0, tFgB = 0;
    int bgR = 0, bgG = 0, bgB = 0;
    int fgR = 0, fgG = 0, fgB = 0;
    int bR = 0, bG = 0, bB = 0;

    render_.parseHex(titleBgColor_, tR, tG, tB);
    render_.parseHex(titleFgColor_, tFgR, tFgG, tFgB);
    render_.parseHex(bgColor_, bgR, bgG, bgB);
    render_.parseHex(fgColor_, fgR, fgG, fgB);
    render_.parseHex(borderColor_, bR, bG, bB);

    std::ostringstream ss;

    ss << renderTitle(w, bR, bG, bB, tR, tG, tB, tFgR, tFgG, tFgB);

    switch (chartMode_) {
        case Pie:
            ss << renderPieMode(w, bgR, bgG, bgB, fgR, fgG, fgB, bR, bG, bB);
            break;
        case Grid:
            ss << renderGridMode(w, bgR, bgG, bgB, fgR, fgG, fgB, bR, bG, bB);
            break;
        default:
            ss << renderBarMode(w, bgR, bgG, bgB, fgR, fgG, fgB, bR, bG, bB);
            break;
    }

    ss << renderBottom(w, bR, bG, bB);

    return ss.str();
}

} // namespace ui