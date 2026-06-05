#include "_component.h"
#include <sstream>
#include <algorithm>

namespace ui {

/**
 * @brief Parse a hex color string into RGB components.
 *
 * Takes a hex color string in the format "#RRGGBB" and extracts the
 * red, green, and blue components as integers.
 *
 * @param hex The hex color string to parse (e.g., "#ff0000" for red).
 * @param r Reference to store the red component (0-255).
 * @param g Reference to store the green component (0-255).
 * @param b Reference to store the blue component (0-255).
 * @return true if parsing was successful, false otherwise.
 */
bool RenderHelper::parseHex(const std::string &hex, int &r, int &g, int &b) const
{
    if (hex.size() != 7 || hex[0] != '#') return false;
    try {
        r = std::stoi(hex.substr(1,2), nullptr, 16);
        g = std::stoi(hex.substr(3,2), nullptr, 16);
        b = std::stoi(hex.substr(5,2), nullptr, 16);
        return true;
    } catch (...) {
        return false;
    }
}

/**
 * @brief Generate an ANSI escape code for the foreground color.
 *
 * Constructs an ANSI 24-bit color escape code for setting the
 * foreground (text) color using the given RGB components.
 *
 * @param r Red component (0-255).
 * @param g Green component (0-255).
 * @param b Blue component (0-255).
 * @return ANSI escape sequence string, e.g. "\x1b[38;2;255;0;0m".
 */
std::string RenderHelper::fg(int r, int g, int b) const
{
    return "\x1b[38;2;" + std::to_string(r) + ';' + std::to_string(g) + ';' + std::to_string(b) + 'm';
}

/**
 * @brief Generate an ANSI escape code for the background color.
 *
 * Constructs an ANSI 24-bit color escape code for setting the
 * background color using the given RGB components.
 *
 * @param r Red component (0-255).
 * @param g Green component (0-255).
 * @param b Blue component (0-255).
 * @return ANSI escape sequence string, e.g. "\x1b[48;2;0;0;255m".
 */
std::string RenderHelper::bg(int r, int g, int b) const
{
    return "\x1b[48;2;" + std::to_string(r) + ';' + std::to_string(g) + ';' + std::to_string(b) + 'm';
}

/**
 * @brief Generate an ANSI escape code to reset terminal colors.
 *
 * Returns the ANSI reset sequence that clears all color and style
 * attributes, reverting to the terminal defaults.
 *
 * @return ANSI reset sequence "\x1b[0m".
 */
std::string RenderHelper::reset() const
{
    return "\x1b[0m";
}

/**
 * @brief Word-wrap text to a specified maximum width.
 *
 * Splits the input text into lines such that no line exceeds
 * maxWidth characters. Words are preserved whole; if a single
 * word exceeds maxWidth it is placed on its own line.
 *
 * @param text The text to wrap.
 * @param maxWidth The maximum number of characters per line.
 * @return A vector of wrapped lines.
 */
std::vector<std::string> RenderHelper::wrapText(const std::string &text, int maxWidth) const
{
    std::vector<std::string> lines;
    if (text.empty()) {
        lines.push_back("");
        return lines;
    }

    std::istringstream stream(text);
    std::string word;
    std::string currentLine;

    while (stream >> word) {
        if (currentLine.empty()) {
            currentLine = word;
        } else if ((int)(currentLine.size() + 1 + word.size()) <= maxWidth) {
            currentLine += ' ' + word;
        } else {
            lines.push_back(currentLine);
            currentLine = word;
        }
    }
    if (!currentLine.empty()) {
        lines.push_back(currentLine);
    }

    return lines;
}

} // namespace ui
