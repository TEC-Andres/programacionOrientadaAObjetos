#include "dialogbox.h"
#include <algorithm>
#include <sstream>

namespace ui {

DialogBox::DialogBox(
    const std::string &title,
    const std::string &message,
    int width,
    int height,
    const std::string &titleBgColor,
    const std::string &titleFgColor,
    const std::string &bodyBgColor,
    const std::string &bodyFgColor,
    const std::string &borderColor,
    Align align)
    : ComponentBase(width, height)
    , title_(title)
    , message_(message)
    , titleBgColor_(titleBgColor)
    , titleFgColor_(titleFgColor)
    , bodyBgColor_(bodyBgColor)
    , bodyFgColor_(bodyFgColor)
    , borderColor_(borderColor)
{
    align_ = align;
}

/**
 * @brief Add a button to the dialog box.
 * @param text The text to display on the button.
 * @param selected Whether the button is initially selected (focused).
 * Example usage:
 * ```cpp
 * DialogBox dialog("Confirm Action", "Are you sure you want to proceed?", 50, 10);
 * dialog.addButton("Yes", true);
 * dialog.addButton("No", false);
 * std::cout << dialog.toString();
 * ```
 */
void DialogBox::addButton(const std::string &text, bool selected)
{
    buttons_.push_back(DialogButton(text, selected));
}

/**
 * @brief Remove all buttons from the dialog box.
 * Example usage:
 * ```cpp
 *  DialogBox dialog("Info", "This is a message.", 40, 8);
 *  dialog.addButton("OK");
 *  std::cout << dialog.toString(); // Dialog with "OK" button
 *  dialog.clearButtons();
 *  std::cout << dialog.toString(); // Dialog without buttons
 * ```
 */
void DialogBox::clearButtons()
{
    buttons_.clear();
}

/**
 * @brief Convert the dialog box to a string representation.
 * This method generates a string that visually represents the dialog box, including its title, message, buttons, and styling based on the specified colors and alignment. The output can be printed to the console to display the dialog box.
 * @return A string representation of the dialog box.
 * Example usage:
 * ```cpp
 * DialogBox dialog("Warning", "This action cannot be undone.", 50, 10);
 * dialog.addButton("Proceed", true);
 * dialog.addButton("Cancel", false);
 * std::cout << dialog.toString();
 * ```
 */
std::string DialogBox::toString() const
{
    int w = width_;
    int h = height_;
    if (w <= 0 || h <= 0) return std::string();
    if (w < 6) w = 6;
    if (h < 6) h = 6;

    int contentWidth = w - 2;
    int overhead = 4;
    int bodyRows = h - overhead;

    int br = 0, bg = 0, bb = 0;
    int fr = 0, fg = 0, fb = 0;
    int bodyBr = 0, bodyBg = 0, bodyBb = 0;
    int bodyFr = 0, bodyFg = 0, bodyFb = 0;
    int borderR = 0, borderG = 0, borderB = 0;

    bool hasTitleBg = render_.parseHex(titleBgColor_, br, bg, bb);
    bool hasTitleFg = render_.parseHex(titleFgColor_, fr, fg, fb);
    bool hasBodyBg = render_.parseHex(bodyBgColor_, bodyBr, bodyBg, bodyBb);
    bool hasBodyFg = render_.parseHex(bodyFgColor_, bodyFr, bodyFg, bodyFb);
    bool hasBorder = render_.parseHex(borderColor_, borderR, borderG, borderB);

    std::ostringstream ss;

    auto borderFg = [&]() -> std::string {
        return hasBorder ? render_.fg(borderR, borderG, borderB) : "";
    };

    ss << borderFg();
    for (int i = 0; i < w; ++i) ss << "█";
    ss << render_.reset() << '\n';

    {
        std::string titleText = title_;
        if ((int)titleText.size() > contentWidth) {
            titleText = titleText.substr(0, contentWidth);
        }
        int padLeft = (contentWidth - (int)titleText.size()) / 2;
        int padRight = contentWidth - (int)titleText.size() - padLeft;

        ss << borderFg() << "█" << render_.reset();
        if (hasTitleBg) ss << render_.bg(br, bg, bb);
        if (hasTitleFg) ss << render_.fg(fr, fg, fb);
        for (int i = 0; i < padLeft; ++i) ss << ' ';
        ss << titleText;
        for (int i = 0; i < padRight; ++i) ss << ' ';
        ss << render_.reset();
        ss << borderFg() << "█" << render_.reset() << '\n';
    }

    ss << borderFg();
    for (int i = 0; i < w; ++i) ss << "█";
    ss << render_.reset() << '\n';

    auto wrappedLines = render_.wrapText(message_, contentWidth);
    int lineCount = (int)wrappedLines.size();
    int buttonRows = buttons_.empty() ? 0 : 1;
    int textRows = bodyRows - buttonRows;

    for (int row = 0; row < textRows; ++row) {
        ss << borderFg() << "█" << render_.reset();
        if (hasBodyBg) ss << render_.bg(bodyBr, bodyBg, bodyBb);
        if (hasBodyFg) ss << render_.fg(bodyFr, bodyFg, bodyFb);

        if (row < lineCount) {
            std::string line = wrappedLines[row];
            ss << line;
            int remaining = contentWidth - (int)line.size();
            for (int i = 0; i < remaining; ++i) ss << ' ';
        } else {
            for (int i = 0; i < contentWidth; ++i) ss << ' ';
        }

        ss << render_.reset();
        ss << borderFg() << "█" << render_.reset() << '\n';
    }

    if (!buttons_.empty()) {
        ss << borderFg() << "█" << render_.reset();
        if (hasBodyBg) ss << render_.bg(bodyBr, bodyBg, bodyBb);
        if (hasBodyFg) ss << render_.fg(bodyFr, bodyFg, bodyFb);
        for (int i = 0; i < contentWidth; ++i) ss << ' ';
        ss << render_.reset();
        ss << borderFg() << "█" << render_.reset() << '\n';

        ss << borderFg() << "█" << render_.reset();

        int buttonCount = (int)buttons_.size();
        int totalBtnWidth = 0;
        for (const auto &b : buttons_) {
            totalBtnWidth += (int)b.text.size() + 4;
        }
        int spacing = buttonCount > 1 ? (contentWidth - totalBtnWidth) / (buttonCount + 1) : (contentWidth - totalBtnWidth) / 2;
        if (spacing < 1) spacing = 1;

        int pos = 0;
        for (int i = 0; i < buttonCount; ++i) {
            if (i == 0) {
                for (int j = 0; j < spacing && pos < contentWidth; ++j, ++pos) {
                    if (hasBodyBg) ss << render_.bg(bodyBr, bodyBg, bodyBb);
                    if (hasBodyFg) ss << render_.fg(bodyFr, bodyFg, bodyFb);
                    ss << ' ';
                }
            }

            const auto &btn = buttons_[i];
            std::string label = btn.text;
            int btnW = (int)label.size() + 4;
            if (pos + btnW > contentWidth) break;

            int btnR = 0, btnG = 0, btnB = 0;
            bool hasBtnBorder = render_.parseHex(borderColor_, btnR, btnG, btnB);
            if (btn.selected) {
                ss << render_.fg(255, 255, 255) << render_.bg(51, 114, 214);
            } else {
                if (hasBtnBorder) ss << render_.fg(btnR, btnG, btnB);
                if (hasBodyBg) ss << render_.bg(bodyBr, bodyBg, bodyBb);
            }
            ss << "█";
            ++pos;

            if (btn.selected) {
                ss << render_.bg(51, 114, 214) << render_.fg(255, 255, 255);
            } else {
                if (hasBodyBg) ss << render_.bg(bodyBr, bodyBg, bodyBb);
                if (hasBodyFg) ss << render_.fg(bodyFr, bodyFg, bodyFb);
            }
            ss << ' ';
            ++pos;

            for (char c : label) {
                ss << c; ++pos;
            }

            ss << ' ';
            ++pos;

            if (btn.selected) {
                ss << render_.fg(255, 255, 255) << render_.bg(51, 114, 214);
            } else {
                if (hasBtnBorder) ss << render_.fg(btnR, btnG, btnB);
                if (hasBodyBg) ss << render_.bg(bodyBr, bodyBg, bodyBb);
            }
            ss << "█";
            ++pos;

            for (int j = 0; j < spacing && pos < contentWidth; ++j, ++pos) {
                if (btn.selected) {
                    ss << render_.reset();
                }
                if (hasBodyBg) ss << render_.bg(bodyBr, bodyBg, bodyBb);
                if (hasBodyFg) ss << render_.fg(bodyFr, bodyFg, bodyFb);
                ss << ' ';
            }
        }

        while (pos < contentWidth) {
            if (hasBodyBg) ss << render_.bg(bodyBr, bodyBg, bodyBb);
            if (hasBodyFg) ss << render_.fg(bodyFr, bodyFg, bodyFb);
            ss << ' '; ++pos;
        }

        ss << render_.reset();
        ss << borderFg() << "█" << render_.reset() << '\n';
    }

    ss << borderFg();
    for (int i = 0; i < w; ++i) ss << "█";
    ss << render_.reset() << '\n';

    return ss.str();
}

} // namespace ui
