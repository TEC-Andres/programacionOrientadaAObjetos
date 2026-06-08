#include "textbox.h"
#include <sstream>

namespace ui {

TextBox::TextBox(
    int maxLength,
    const std::string &bgColor,
    const std::string &fgColor,
    const std::string &borderColor,
    bool passwordMode,
    Align align)
    : ComponentBase(30, 3)
    , maxLength_(maxLength)
    , bgColor_(bgColor)
    , fgColor_(fgColor)
    , borderColor_(borderColor)
    , passwordMode_(passwordMode)
    , selected_(false)
{
    align_ = align;
}

bool TextBox::handleKey(int key)
{
    if (!selected_) return false;

    // Enter submits the current text
    if (key == 13 || key == 10) {
        if (onSubmit_) onSubmit_(text_);
        return true;
    }

    // Escape — not handled, falls through to MapComponent
    if (key == 27) {
        return false;
    }

    // Backspace deletes last character
    if (key == 8 || key == 127) {
        if (!text_.empty()) {
            text_.pop_back();
            if (onChange_) onChange_(text_);
        }
        return true;
    }

    // Printable characters (space through ~)
    if (key >= 32 && key <= 126) {
        if ((int)text_.size() < maxLength_) {
            text_ += (char)key;
            if (onChange_) onChange_(text_);
        }
        return true;
    }

    return false;
}

std::string TextBox::toString() const
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
        activeBg = "#ffffff";
        activeFg = "#000000";
        activeBorder = "#00cc66";
    }

    bool hasBg = render_.parseHex(activeBg, br, bg, bb);
    bool hasFg = render_.parseHex(activeFg, fr, fg, fb);
    bool hasBorder = render_.parseHex(activeBorder, borderR, borderG, borderB);

    std::ostringstream ss;

    auto borderFg = [&]() -> std::string {
        return hasBorder ? render_.fg(borderR, borderG, borderB) : "";
    };

    // Build display text: mask with '*' if password mode
    std::string displayText = passwordMode_
        ? std::string(text_.size(), '*')
        : text_;

    // Append cursor indicator when selected
    if (selected_) {
        displayText += '|';
    }

    // Trunking: show the LAST contentWidth characters
    if ((int)displayText.size() > contentWidth) {
        displayText = displayText.substr(displayText.size() - contentWidth);
    }

    // Line 1: top border
    ss << borderFg();
    for (int i = 0; i < w; ++i) ss << "█";
    ss << render_.reset() << '\n';

    // Line 2: content
    ss << borderFg() << "█";
    ss << render_.reset();
    if (hasBg) ss << render_.bg(br, bg, bb);
    if (hasFg) ss << render_.fg(fr, fg, fb);

    // Left-align text in content area
    int textLen = (int)displayText.size();
    for (int i = 0; i < textLen; ++i) ss << displayText[i];
    for (int i = textLen; i < contentWidth; ++i) ss << ' ';

    ss << render_.reset();
    ss << borderFg() << "█";
    ss << render_.reset() << '\n';

    // Line 3: bottom border
    ss << borderFg();
    for (int i = 0; i < w; ++i) ss << "█";
    ss << render_.reset() << '\n';

    return ss.str();
}

} // namespace ui
