#include "messagebox.h"
#include <sstream>

namespace ui {

MessageBox::MessageBox(
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
    , title_(title),
      message_(message),
      titleBgColor_(titleBgColor),
      titleFgColor_(titleFgColor),
      bodyBgColor_(bodyBgColor),
      bodyFgColor_(bodyFgColor),
      borderColor_(borderColor)
{
    align_ = align;
}

std::string MessageBox::toString() const
{
    int w = width_;
    int h = height_;
    if (w <= 0 || h <= 0) return std::string();
    if (w < 4) w = 4;
    if (h < 3) h = 3;

    int contentWidth = w - 2;
    int bodyHeight = h - 2;

    int br=0, bg=0, bb=0;
    int fr=0, fg=0, fb=0;
    int bodyBr=0, bodyBg=0, bodyBb=0;
    int bodyFr=0, bodyFg=0, bodyFb=0;
    int borderR=0, borderG=0, borderB=0;

    bool hasTitleBg = render_.parseHex(titleBgColor_, br, bg, bb);
    bool hasTitleFg = render_.parseHex(titleFgColor_, fr, fg, fb);
    bool hasBodyBg = render_.parseHex(bodyBgColor_, bodyBr, bodyBg, bodyBb);
    bool hasBodyFg = render_.parseHex(bodyFgColor_, bodyFr, bodyFg, bodyFb);
    bool hasBorder = render_.parseHex(borderColor_, borderR, borderG, borderB);

    std::ostringstream ss;

    auto borderFg = [&]() -> std::string {
        return hasBorder ? render_.fg(borderR, borderG, borderB) : "";
    };

    ss << borderFg() << "█";
    for (int i = 0; i < contentWidth; ++i) ss << "█";
    ss << "█";
    ss << render_.reset() << '\n';

    {
        std::string titleText = title_;
        if ((int)titleText.size() > contentWidth) {
            titleText = titleText.substr(0, contentWidth);
        }
        int padLeft = (contentWidth - (int)titleText.size()) / 2;
        int padRight = contentWidth - (int)titleText.size() - padLeft;

        ss << borderFg() << "█";
        ss << render_.reset();
        if (hasTitleBg || hasTitleFg) {
            if (hasTitleBg) ss << render_.bg(br, bg, bb);
            if (hasTitleFg) ss << render_.fg(fr, fg, fb);
        }
        for (int i = 0; i < padLeft; ++i) ss << ' ';
        ss << titleText;
        for (int i = 0; i < padRight; ++i) ss << ' ';
        ss << render_.reset();
        ss << borderFg() << "█";
        ss << render_.reset() << '\n';
    }

    ss << borderFg() << "█";
    for (int i = 0; i < contentWidth; ++i) ss << "█";
    ss << "█";
    ss << render_.reset() << '\n';

    auto wrappedLines = render_.wrapText(message_, contentWidth);
    int lineCount = (int)wrappedLines.size();

    for (int row = 0; row < bodyHeight; ++row) {
        ss << borderFg() << "█";
        ss << render_.reset();
        if (hasBodyBg || hasBodyFg) {
            if (hasBodyBg) ss << render_.bg(bodyBr, bodyBg, bodyBb);
            if (hasBodyFg) ss << render_.fg(bodyFr, bodyFg, bodyFb);
        }

        if (row < lineCount) {
            std::string line = wrappedLines[row];
            ss << line;
            int remaining = contentWidth - (int)line.size();
            for (int i = 0; i < remaining; ++i) ss << ' ';
        } else {
            for (int i = 0; i < contentWidth; ++i) ss << ' ';
        }

        ss << render_.reset();
        ss << borderFg() << "█";
        ss << render_.reset() << '\n';
    }

    ss << borderFg() << "█";
    for (int i = 0; i < contentWidth; ++i) ss << "█";
    ss << "█";
    ss << render_.reset() << '\n';

    return ss.str();
}

} // namespace ui
