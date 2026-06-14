#include "terminalArea.h"
#include <sstream>
#include <algorithm>

namespace ui {

TerminalArea::TerminalArea(
    int width,
    int height,
    const std::string &bgColor,
    const std::string &fgColor,
    const std::string &borderColor,
    const std::string &promptColor,
    const std::string &outputColor,
    const std::string &errorColor)
    : ComponentBase(width, height)
    , bgColor_(bgColor)
    , fgColor_(fgColor)
    , borderColor_(borderColor)
    , promptColor_(promptColor)
    , outputColor_(outputColor)
    , errorColor_(errorColor)
{
    align_ = Align::TopLeft;
    addOutput("Terminal ready. Type :help for commands.");
}

void TerminalArea::addOutput(const std::string &text, bool isError)
{
    lines_.push_back(Line(text, isError, false));
    if ((int)lines_.size() > maxLines_) {
        lines_.pop_front();
    }
    setDirty();
}

void TerminalArea::addCommand(const std::string &cmd)
{
    lines_.push_back(Line("> " + cmd, false, true));
    if ((int)lines_.size() > maxLines_) {
        lines_.pop_front();
    }
    setDirty();
}

void TerminalArea::clear()
{
    lines_.clear();
    currentInput_.clear();
    setDirty();
}

bool TerminalArea::handleKey(int key)
{
    if (!selected_) return false;

    if (key >= 32 && key <= 126) {
        currentInput_ += (char)key;
        setDirty();
        return true;
    }

    if (key == 8 || key == 127) {
        if (!currentInput_.empty()) {
            currentInput_.pop_back();
            setDirty();
        }
        return true;
    }

    if (key == 13 || key == 10) {
        if (!currentInput_.empty()) {
            std::string cmd = currentInput_;
            if (onCommand_) onCommand_(cmd);
            currentInput_.clear();
            setDirty();
        }
        return true;
    }

    return false;
}

std::string TerminalArea::toString() const
{
    int w = width_;
    if (w < 12) w = 12;

    int bgR = 0, bgG = 0, bgB = 0;
    int fgR = 0, fgG = 0, fgB = 0;
    int bR = 0, bG = 0, bB = 0;
    int pR = 0, pG = 0, pB = 0;
    int oR = 0, oG = 0, oB = 0;
    int eR = 0, eG = 0, eB = 0;

    render_.parseHex(bgColor_, bgR, bgG, bgB);
    render_.parseHex(fgColor_, fgR, fgG, fgB);
    render_.parseHex(borderColor_, bR, bG, bB);
    render_.parseHex(promptColor_, pR, pG, pB);
    render_.parseHex(outputColor_, oR, oG, oB);
    render_.parseHex(errorColor_, eR, eG, eB);

    std::ostringstream ss;
    auto borderFg = [&]() -> std::string { return render_.fg(bR, bG, bB); };
    auto reset = [&]() -> std::string { return render_.reset(); };

    int maxRows = height_;
    if (maxRows < 3) maxRows = 3;
    int contentRows = maxRows;
    bool showInput = selected_;

    if (showInput) {
        contentRows = maxRows - 3;
        if (contentRows < 1) contentRows = 1;
    }

    // Top border
    ss << borderFg() << "┌";
    for (int i = 0; i < w - 2; ++i) ss << "─";
    ss << "┐" << reset() << '\n';

    // Determine visible lines (from end)
    int totalLines = (int)lines_.size();
    int startLine = std::max(0, totalLines - contentRows);

    // Fill background for unused rows
    int unusedTop = contentRows - std::min(contentRows, totalLines - startLine);
    for (int r = 0; r < unusedTop; ++r) {
        ss << borderFg() << "│" << reset();
        ss << render_.bg(bgR, bgG, bgB);
        for (int i = 0; i < w - 2; ++i) ss << ' ';
        ss << reset();
        ss << borderFg() << "│" << reset() << '\n';
    }

    // Render lines
    for (int i = startLine; i < totalLines; ++i) {
        ss << borderFg() << "│" << reset();
        const Line &line = lines_[i];

        int useFgR = fgR, useFgG = fgG, useFgB = fgB;
        if (line.isPrompt) {
            useFgR = pR; useFgG = pG; useFgB = pB;
        } else if (line.isError) {
            useFgR = eR; useFgG = eG; useFgB = eB;
        } else {
            useFgR = oR; useFgG = oG; useFgB = oB;
        }

        ss << render_.bg(bgR, bgG, bgB) << render_.fg(useFgR, useFgG, useFgB);

        std::string text = line.text;
        if ((int)text.size() > w - 2) {
            text = text.substr(0, w - 5) + "...";
        }
        ss << text;
        for (int j = (int)text.size(); j < w - 2; ++j) ss << ' ';
        ss << reset();
        ss << borderFg() << "│" << reset() << '\n';
    }

    // Input prompt line (only when selected)
    if (showInput) {
        ss << borderFg() << "│" << reset();
        ss << render_.bg(bgR, bgG, bgB) << render_.fg(pR, pG, pB);
        std::string prompt = "> " + currentInput_;
        if ((int)prompt.size() > w - 2) {
            prompt = prompt.substr(prompt.size() - (w - 2));
        }
        ss << prompt;
        for (int j = (int)prompt.size(); j < w - 2; ++j) ss << ' ';
        ss << reset();
        ss << borderFg() << "│" << reset() << '\n';
    }

    // Bottom border
    ss << borderFg() << "└";
    for (int i = 0; i < w - 2; ++i) ss << "─";
    ss << "┘" << reset() << '\n';

    return ss.str();
}

int TerminalArea::cursorX() const
{
    int w = width_;
    if (w < 12) w = 12;
    int contentW = w - 2;
    int promptLen = 2 + (int)currentInput_.size();
    if (promptLen >= contentW) promptLen = contentW - 1;
    return 1 + promptLen;
}

int TerminalArea::cursorY() const
{
    int h = height_;
    if (h < 3) h = 3;
    return h - 2;
}

} // namespace ui
