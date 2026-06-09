#include "button.h"
#include <sstream>

namespace ui {

Button::Button(
    const std::string &text,
    int width,
    const std::string &bgColor,
    const std::string &fgColor,
    const std::string &borderColor,
    bool selected,
    Align align)
    : ComponentBase(width, 3)
    , text_(text),
      bgColor_(bgColor),
      fgColor_(fgColor),
      borderColor_(borderColor),
      selected_(selected)
{
    align_ = align;
}

/**
 * @brief Convert the button to a string representation.
 * This method generates a string that visually represents the button, including its text, colors, and styling based on the specified parameters. The output can be printed to the console to display the button.
 * @return A string representation of the button.
 */
std::string Button::toString() const
{
    int w = width_;
    if (w < 6) w = 6;

    int contentWidth = w - 2;

    int br = 0, bg = 0, bb = 0;
    int fr = 0, fg = 0, fb = 0;
    int borderR = 0, borderG = 0, borderB = 0;

    std::string activeBg = bgColor_;
    std::string activeFg = fgColor_;
    std::string activeBorder = borderColor_;

    if (selected_) {
        activeBg = "#3366cc";
        activeFg = "#ffffff";
        activeBorder = "#ffffff";
    }

    bool hasBg = render_.parseHex(activeBg, br, bg, bb);
    bool hasFg = render_.parseHex(activeFg, fr, fg, fb);
    bool hasBorder = render_.parseHex(activeBorder, borderR, borderG, borderB);

    std::ostringstream ss;

    auto borderFg = [&]() -> std::string {
        return hasBorder ? render_.fg(borderR, borderG, borderB) : "";
    };

    ss << borderFg();
    for (int i = 0; i < w; ++i) ss << "▄";
    ss << render_.reset() << '\n';

    ss << borderFg() << "█";
    ss << render_.reset();
    if (hasBg) ss << render_.bg(br, bg, bb);
    if (hasFg) ss << render_.fg(fr, fg, fb);

    std::string label = text_;
    if ((int)label.size() > contentWidth) {
        label = label.substr(0, contentWidth);
    }
    int padLeft = (contentWidth - (int)label.size()) / 2;
    int padRight = contentWidth - (int)label.size() - padLeft;

    for (int i = 0; i < padLeft; ++i) ss << ' ';
    ss << label;
    for (int i = 0; i < padRight; ++i) ss << ' ';

    ss << render_.reset();
    ss << borderFg() << "█";
    ss << render_.reset() << '\n';

    ss << render_.reset();
    ss << borderFg();
    for (int i = 0; i < w; ++i) ss << "▀";
    ss << render_.reset() << '\n';

    return ss.str();
}

} // namespace ui
