#include "textbox.h"
#include <sstream>

namespace ui {

TextBox::TextBox(
    int maxLength,
    const std::string &bgColor,
    const std::string &fgColor,
    const std::string &borderColor,
    bool passwordMode,
    Align align,
    const std::string &ghostMessage,
    const std::string &ghostColor)
    : ComponentBase(30, 3)
    , maxLength_(maxLength)
    , bgColor_(bgColor)
    , fgColor_(fgColor)
    , borderColor_(borderColor)
    , passwordMode_(passwordMode)
    , selected_(false)
    , ghostMessage_(ghostMessage)
    , ghostColor_(ghostColor)
{
    align_ = align;
}

/**
 * @brief Handle key input for the TextBox component.
 * This method processes key events when the TextBox is selected. It supports character input, backspace for deletion, and submission with the Enter key. The Escape key is not handled here and will be passed to the parent component.
 * @param key The integer code of the key that was pressed.
 * @return true if the key event was handled, false otherwise.
 */
bool TextBox::handleKey(int key)
{
    if (!selected_) return false;

    // Enter is handled but does nothing — no activation
    if (key == 13 || key == 10) {
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

/**
 * @brief Convert the TextBox component to a string representation.
 * This method generates a string that visually represents the TextBox, including its borders and content. It takes into account the current text, whether the TextBox is in password mode (masking input with '*'), and the selected state (which may change colors). The output can be printed to the console to display the TextBox.
 * @return A string representing the TextBox.
 */
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

    // Determine if we should show ghost text
    bool showGhost = text_.empty() && !ghostMessage_.empty() && !selected_;

    // Build display text: mask with '*' if password mode
    std::string displayText;
    if (showGhost) {
        displayText = ghostMessage_;
    } else {
        displayText = passwordMode_
            ? std::string(text_.size(), '*')
            : text_;
    }

    // Append cursor indicator when selected (not when showing ghost)
    if (selected_ && !showGhost) {
        displayText += '|';
    }

    // Trunking: show the LAST contentWidth characters
    if ((int)displayText.size() > contentWidth) {
        displayText = displayText.substr(displayText.size() - contentWidth);
    }

    // Line 1: top border
    ss << borderFg();
    for (int i = 0; i < w; ++i) ss << "▄";
    ss << render_.reset() << '\n';

    // Line 2: content
    ss << borderFg() << "█";
    ss << render_.reset();
    if (hasBg) ss << render_.bg(br, bg, bb);

    // Use ghost color for ghost text, normal fg otherwise
    if (showGhost) {
        int gr = 0, gg = 0, gb = 0;
        bool hasGhost = render_.parseHex(ghostColor_, gr, gg, gb);
        if (hasGhost) ss << render_.fg(gr, gg, gb);
    } else {
        if (hasFg) ss << render_.fg(fr, fg, fb);
    }

    // Left-align text in content area
    int textLen = (int)displayText.size();
    for (int i = 0; i < textLen; ++i) ss << displayText[i];
    for (int i = textLen; i < contentWidth; ++i) ss << ' ';

    ss << render_.reset();
    ss << borderFg() << "█";
    ss << render_.reset() << '\n';

    // Line 3: bottom border
    ss << render_.reset();
    ss << borderFg();
    for (int i = 0; i < w; ++i) ss << "▀";
    ss << render_.reset() << '\n';

    return ss.str();
}

} // namespace ui
