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
    : ComponentBase(width, 1)
    , bgColor_(bgColor)
    , fgColor_(fgColor)
{
    segments_.reserve(3);
    segments_.emplace_back(segment1, text1, location1);
    segments_.emplace_back(segment2, text2, location2);
    segments_.emplace_back(segment3, text3, location3);
}

/**
 * @brief Place the given text into the buffer at the specified position, ensuring it does not overflow the buffer.
 * This helper function is used to insert segment text into the location bar's buffer at the correct position based on alignment. It checks for empty text and ensures that the text fits within the bounds of the buffer, preventing overflow.
 * @param buf The buffer string where the text will be placed.
 * @param txt The text to be placed into the buffer.
 * @param pos The starting position in the buffer where the text should be placed.
 */
static void placeText(std::string &buf, const std::string &txt, int pos)
{
    if (txt.empty()) return;
    if (pos < 0) pos = 0;
    int maxCopy = std::max(0, (int)buf.size() - pos);
    int copyLen = std::min((int)txt.size(), maxCopy);
    for (int i = 0; i < copyLen; ++i) buf[pos + i] = txt[i];
}

/**
 * @brief Convert the location bar to a string representation.
 * This method generates a string that visually represents the location bar, including its segments and styling based on the specified parameters. The output can be printed to the console to display the location bar.
 * @return A string representation of the location bar.
 */
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
            case Align::TopLeft:
                pos = 0;
                break;
            case Align::Center:
            case Align::TopCenter:
                pos = (width_ - len) / 2;
                break;
            case Align::Right:
            case Align::TopRight:
                pos = width_ - len;
                break;
            default:
                pos = 0;
                break;
        }
        if (pos < 0) pos = 0;
        placeText(buf, seg.text, pos);
    }

    return buf;
}

/**
 * @brief Render the location bar to the specified output stream.
 * This method generates the string representation of the location bar and outputs it to the provided stream. It ensures that the rendering is done with the correct console width.
 * @param out The output stream where the location bar will be rendered.
 */
void LocationBar::render(std::ostream &out)
{
    int consW = (width_ > 0) ? width_ : ComponentBase::getConsoleWidth();
    if (consW <= 0) consW = 80;

    int oldW = width_;
    width_ = consW;
    std::string line = toString();
    width_ = oldW;

    int br=0, bg=0, bb=0;
    int fr=0, fg=0, fb=0;
    bool hasBg = render_.parseHex(bgColor_, br, bg, bb);
    bool hasFg = render_.parseHex(fgColor_, fr, fg, fb);

    std::ostringstream ss;
    if (hasBg) ss << render_.bg(br, bg, bb);
    if (hasFg) ss << render_.fg(fr, fg, fb);
    ss << line;
    ss << render_.reset() << std::endl;

    out << ss.str();
}

} // namespace ui
