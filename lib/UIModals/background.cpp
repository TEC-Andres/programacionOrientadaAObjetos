#include "background.h"
#include <sstream>

namespace ui {

Background::Background(const std::string &hexColor)
    : color_(hexColor)
{
}

/**
 * @brief Render the background component as a string of ANSI escape codes that fill the console with the specified background color.
 * The toString() method generates a string that consists of multiple lines, each line containing spaces with the background color applied. The number of lines and the width of each line are determined by the current console dimensions. The method uses the RenderHelper to parse the hex color and generate the appropriate ANSI escape codes for setting the background color. If the console dimensions cannot be determined, it returns an empty string.
 * @return A string representation of the background component.
 */
std::string Background::toString() const
{
    int w = getConsoleWidth();
    int h = getConsoleHeight();
    if (w <= 0 || h <= 0) return "";

    int r = 0, g = 0, b = 0;
    std::string bgCode;
    if (render_.parseHex(color_, r, g, b)) {
        bgCode = render_.bg(r, g, b);
    }

    std::ostringstream ss;
    std::string line(w, ' ');
    for (int i = 0; i < h; ++i) {
        ss << bgCode << line << render_.reset();
        if (i < h - 1) ss << '\n';
    }
    return ss.str();
}

} // namespace ui
