#include "_component.h"
#include <sstream>
#include <algorithm>

namespace ui {

/**
 * @brief Parse a hex color string into RGB components.
 * 
 * This method takes a hex color string in the format "#RRGGBB" and extracts the
 * red, green, and blue components as integers. It returns `true` if the parsing
 * was successful, and `false` if the input string is not in the correct format or contains invalid characters.
 * @param hex The hex color string to parse (e.g., "#ff0000" for red).
 * @param r Reference to an integer where the red component will be stored.
 * @param g Reference to an integer where the green component will be stored.
 * @param b Reference to an integer where the blue component will be stored.
 * @return `true` if parsing was successful, `false` otherwise.
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
 * @brief Generate an ANSI escape code for setting the foreground color.
 * This method constructs an ANSI escape code string that can be used to set the foreground color in a terminal. The color is specified by the RGB components provided as arguments.
 * @param r The red component of the color (0-255).
 * @param g The green component of the color (0-255).
 * @param b The blue component of the color (0-255).
 * @return A string containing the ANSI escape code for the specified foreground color.
 */
std::string RenderHelper::fg(int r, int g, int b) const
{
    return "\x1b[38;2;" + std::to_string(r) + ';' + std::to_string(g) + ';' + std::to_string(b) + 'm';
}

/**
 * @brief Generate an ANSI escape code for setting the background color.
 * This method constructs an ANSI escape code string that can be used to set the background color in a terminal. The color is specified by the RGB components provided as arguments.
 * @param r The red component of the color (0-255).
 * @param g The green component of the color (0-255).
 * @param b The blue component of the color (0-255).
 * @return A string containing the ANSI escape code for the specified background color.
 */
std::string RenderHelper::bg(int r, int g, int b) const
{
    return "\x1b[48;2;" + std::to_string(r) + ';' + std::to_string(g) + ';' + std::to_string(b) + 'm';
}

/**
 * @brief Generate an ANSI escape code for resetting the terminal color.
 * This method constructs an ANSI escape code string that can be used to reset the terminal color to its default.
 * @return A string containing the ANSI escape code for resetting the terminal color.
 */
std::string RenderHelper::reset() const
{
    return "\x1b[0m";
}

/**
 * @brief Wrap text to a specified maximum width.
 * This method wraps the input text to ensure that no line exceeds the specified maximum width.
 * @param text The text to wrap.
 * @param maxWidth The maximum width of each line.
 * @return A vector of strings, where each string represents a line of wrapped text.
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
