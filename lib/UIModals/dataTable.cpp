#include "dataTable.h"
#include <sstream>
#include <algorithm>
#include <cmath>

namespace ui {

static int displayWidth(const std::string &s) {
    int w = 0;
    for (size_t i = 0; i < s.size(); ) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        if (c < 0x80) {
            w += 1;
            i += 1;
        } else if ((c & 0xE0) == 0xC0) {
            w += 1;
            i += 2;
        } else if ((c & 0xF0) == 0xE0) {
            w += 1;
            i += 3;
        } else if ((c & 0xF8) == 0xF0) {
            w += 2;
            i += 4;
        } else {
            w += 1;
            i += 1;
        }
    }
    return w;
}

static std::string truncateToWidth(const std::string &s, int maxW) {
    int w = 0;
    size_t lastGood = 0;
    for (size_t i = 0; i < s.size(); ) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        int charW = 0;
        size_t charLen = 0;
        if (c < 0x80) { charW = 1; charLen = 1; }
        else if ((c & 0xE0) == 0xC0) { charW = 1; charLen = 2; }
        else if ((c & 0xF0) == 0xE0) { charW = 1; charLen = 3; }
        else if ((c & 0xF8) == 0xF0) { charW = 2; charLen = 4; }
        else { charW = 1; charLen = 1; }
        if (w + charW > maxW - 1) break;
        w += charW;
        lastGood = i + charLen;
        i += charLen;
    }
    return s.substr(0, lastGood) + "\xe2\x80\xa6";
}

DataTable::DataTable(
    int width,
    int height,
    const std::string &bgColor,
    const std::string &fgColor,
    const std::string &borderColor,
    const std::string &headerBgColor,
    const std::string &headerFgColor,
    const std::string &altRowColor,
    int maxRows)
    : ComponentBase(width, height)
    , bgColor_(bgColor)
    , fgColor_(fgColor)
    , borderColor_(borderColor)
    , headerBgColor_(headerBgColor)
    , headerFgColor_(headerFgColor)
    , altRowColor_(altRowColor)
    , maxRows_(maxRows)
{
    align_ = Align::TopLeft;
}

void DataTable::addRow(const TableRow &row)
{
    rows_.push_back(row);
}

int DataTable::totalPages() const
{
    if (rows_.empty() || maxRows_ <= 0) return 1;
    return (int)std::ceil((double)rows_.size() / maxRows_);
}

void DataTable::nextPage()
{
    int tp = totalPages();
    if (currentPage_ < tp - 1) {
        currentPage_++;
        setDirty();
    }
}

void DataTable::prevPage()
{
    if (currentPage_ > 0) {
        currentPage_--;
        setDirty();
    }
}

bool DataTable::handleKey(int key)
{
    if (!selected_) return false;

    if (key == UI_KEY_UP) {
        if (selectedRow_ > 0) {
            selectedRow_--;
            // Check if we need to go to previous page
            int pageStart = currentPage_ * maxRows_;
            if (selectedRow_ < pageStart) {
                currentPage_--;
                setDirty();
            }
            setDirty();
        }
        return true;
    }

    if (key == UI_KEY_DOWN) {
        if (selectedRow_ < (int)rows_.size() - 1) {
            selectedRow_++;
            // Check if we need to go to next page
            int pageEnd = (currentPage_ + 1) * maxRows_ - 1;
            if (selectedRow_ > pageEnd) {
                currentPage_++;
                setDirty();
            }
            setDirty();
        }
        return true;
    }

    if (key == UI_KEY_LEFT) {
        if (currentPage_ > 0) {
            currentPage_--;
            selectedRow_ = currentPage_ * maxRows_;
            setDirty();
        }
        return true;
    }

    if (key == UI_KEY_RIGHT) {
        if (currentPage_ < totalPages() - 1) {
            currentPage_++;
            selectedRow_ = currentPage_ * maxRows_;
            setDirty();
        }
        return true;
    }

    if (key == 13 || key == 10) {
        if (onActivate_) onActivate_();
        return true;
    }

    return false;
}

std::vector<int> DataTable::computeColumnWidths(int availWidth) const
{
    std::vector<int> widths;
    if (headers_.empty()) return widths;

    int n = (int)headers_.size();
    widths.resize(n, 0);

    for (int i = 0; i < n; ++i) {
        widths[i] = displayWidth(headers_[i]);
    }
    for (const auto &row : rows_) {
        for (int i = 0; i < n && i < (int)row.columns.size(); ++i) {
            int len = displayWidth(row.columns[i]);
            if (len > widths[i]) widths[i] = len;
        }
    }

    int total = 0;
    for (int i = 0; i < n; ++i) total += widths[i];

    int sepCount = n - 1;
    if (total + sepCount > availWidth) {
        while (total + sepCount > availWidth) {
            int maxIdx = 0;
            for (int i = 1; i < n; ++i) {
                if (widths[i] > widths[maxIdx]) maxIdx = i;
            }
            if (widths[maxIdx] > 3) {
                widths[maxIdx]--;
                total--;
            } else break;
        }
    } else if (total + sepCount < availWidth) {
        int extra = availWidth - total - sepCount;
        int maxIdx = 0;
        for (int i = 1; i < n; ++i) {
            if (widths[i] > widths[maxIdx]) maxIdx = i;
        }
        widths[maxIdx] += extra;
    }

    return widths;
}

std::string DataTable::toString() const
{
    int w = width_;
    if (w < 12) w = 12;

    int bgR = 0, bgG = 0, bgB = 0;
    int fgR = 0, fgG = 0, fgB = 0;
    int bR = 0, bG = 0, bB = 0;
    int hBgR = 0, hBgG = 0, hBgB = 0;
    int hFgR = 0, hFgG = 0, hFgB = 0;
    int altR = 0, altG = 0, altB = 0;

    render_.parseHex(bgColor_, bgR, bgG, bgB);
    render_.parseHex(fgColor_, fgR, fgG, fgB);
    render_.parseHex(borderColor_, bR, bG, bB);
    render_.parseHex(headerBgColor_, hBgR, hBgG, hBgB);
    render_.parseHex(headerFgColor_, hFgR, hFgG, hFgB);
    render_.parseHex(altRowColor_, altR, altG, altB);

    std::ostringstream ss;
    auto borderFg = [&]() -> std::string { return render_.fg(bR, bG, bB); };
    auto reset = [&]() -> std::string { return render_.reset(); };

    if (headers_.empty()) return ss.str();

    auto widths = computeColumnWidths(w - 2);
    if (widths.empty()) return ss.str();

    int nCols = (int)widths.size();
    int sepCount = nCols - 1;
    int totalWidth = 0;
    for (int i = 0; i < nCols; ++i) totalWidth += widths[i];
    totalWidth += sepCount;

    auto renderSeparator = [&](const std::string &left, const std::string &mid, const std::string &right, const std::string &hor) {
        ss << borderFg() << left;
        for (int i = 0; i < nCols; ++i) {
            for (int j = 0; j < widths[i]; ++j) ss << hor;
            if (i < nCols - 1) ss << mid;
        }
        ss << right << reset() << '\n';
    };

    auto renderCell = [&](const std::string &text, int colWidth, int cellBgR, int cellBgG, int cellBgB,
                           int cellFgR, int cellFgG, int cellFgB) {
        ss << render_.bg(cellBgR, cellBgG, cellBgB) << render_.fg(cellFgR, cellFgG, cellFgB);
        std::string t = text;
        if (displayWidth(t) > colWidth) {
            t = truncateToWidth(t, colWidth);
        }
        ss << t;
        int pad = colWidth - displayWidth(t);
        for (int i = 0; i < pad; ++i) ss << ' ';
        ss << reset();
    };

    // Top border
    renderSeparator("\xe2\x94\x8c", "\xe2\x94\xac", "\xe2\x94\x90", "\xe2\x94\x80");

    // Header row
    ss << borderFg() << "\xe2\x94\x82" << reset();
    for (int i = 0; i < nCols; ++i) {
        renderCell(headers_[i], widths[i], hBgR, hBgG, hBgB, hFgR, hFgG, hFgB);
        ss << borderFg() << "│" << reset();
    }
    ss << '\n';

    // Header-data separator
    renderSeparator("\xe2\x94\x9c", "\xe2\x94\xbc", "\xe2\x94\xa4", "\xe2\x94\x80");

    // Data rows (from current page)
    int startIdx = currentPage_ * maxRows_;
    int endIdx = std::min(startIdx + maxRows_, (int)rows_.size());

    for (int r = startIdx; r < endIdx; ++r) {
        bool isAlt = (r - startIdx) % 2 == 1;
        int cellBgRes = isAlt ? altR : bgR;
        int cellBgGres = isAlt ? altG : bgG;
        int cellBgBres = isAlt ? altB : bgB;
        int cellFgR = fgR, cellFgG = fgG, cellFgB = fgB;

        if (selected_ && r == selectedRow_) {
            cellFgR = 255; cellFgG = 255; cellFgB = 255;
        }

        ss << borderFg() << "│" << reset();
        const auto &row = rows_[r];
        for (int i = 0; i < nCols; ++i) {
            std::string cellText;
            if (i < (int)row.columns.size()) {
                cellText = row.columns[i];
            }
            renderCell(cellText, widths[i], cellBgRes, cellBgGres, cellBgBres, cellFgR, cellFgG, cellFgB);
            ss << borderFg() << "│" << reset();
        }
        ss << '\n';
    }

    // Fill remaining rows
    int usedRows = endIdx - startIdx;
    for (int r = usedRows; r < maxRows_; ++r) {
        if (1 + r >= height_) break;
        bool isAlt = r % 2 == 1;
        int cellBgRes = isAlt ? altR : bgR;
        int cellBgGres = isAlt ? altG : bgG;
        int cellBgBres = isAlt ? altB : bgB;

        ss << borderFg() << "│" << reset();
        for (int i = 0; i < nCols; ++i) {
            renderCell("", widths[i], cellBgRes, cellBgGres, cellBgBres, fgR, fgG, fgB);
            ss << borderFg() << "│" << reset();
        }
        ss << '\n';
    }

    // Bottom border
    renderSeparator("\xe2\x94\x94", "\xe2\x94\xb4", "\xe2\x94\x98", "\xe2\x94\x80");

    // Page indicator
    if (totalPages() > 1) {
        ss << render_.bg(bgR, bgG, bgB) << render_.fg(hFgR, hFgG, hFgB);
        std::string pageStr = " Page " + std::to_string(currentPage_ + 1) + "/" + std::to_string(totalPages()) + " ";
        int pageLen = (int)pageStr.size();
        int pagePad = (w - pageLen) / 2;
        if (pagePad < 0) pagePad = 0;
        for (int i = 0; i < pagePad; ++i) ss << ' ';
        ss << pageStr;
        ss << reset() << '\n';
    }

    return ss.str();
}

} // namespace ui
