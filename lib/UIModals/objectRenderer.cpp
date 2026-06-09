#include "objectRenderer.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace ui {

ObjectRenderer::ObjectRenderer(const std::string &filepath)
    : ComponentBase(0, 0)
    , maxWidth_(0)
    , filepath_(filepath)
{
    if (!filepath_.empty()) {
        loadFile();
    }
}

ObjectRenderer::ObjectRenderer(const char* const* rawLines, int lineCount)
    : ComponentBase(0, 0)
    , maxWidth_(0)
{
    lines_.reserve(lineCount);
    for (int i = 0; i < lineCount; ++i) {
        std::string line(rawLines[i]);
        lines_.push_back(line);
        int w = visibleWidth(line);
        if (w > maxWidth_) maxWidth_ = w;
    }
    width_ = maxWidth_;
    height_ = (int)lines_.size();
}

/**
 * @brief Set the file path for the object renderer.
 * This method updates the file path and reloads the file content.
 * @param filepath The path to the file to be loaded.
 */
void ObjectRenderer::setFile(const std::string &filepath)
{
    filepath_ = filepath;
    loadFile();
}

/**
 * @brief Load the content of the specified file.
 * This method reads the file line by line and stores it in the internal buffer.
 */
void ObjectRenderer::loadFile()
{
    lines_.clear();
    maxWidth_ = 0;

    std::string paths[] = {
        filepath_,
        "../" + filepath_,
    };

    std::ifstream file;
    for (const auto &p : paths) {
        file.open(p);
        if (file.is_open()) {
            filepath_ = p;
            break;
        }
    }

    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        lines_.push_back(line);
        int w = visibleWidth(line);
        if (w > maxWidth_) maxWidth_ = w;
    }
    width_ = maxWidth_;
    height_ = (int)lines_.size();
}

/**
 * @brief Strip ANSI escape codes from the given line.
 * This method removes any ANSI escape codes from the input string, returning only the visible characters.
 * @param line The input string that may contain ANSI escape codes.
 * @return A string with ANSI escape codes removed.
*/
std::string ObjectRenderer::stripAnsi(const std::string &line) const
{
    std::string result;
    result.reserve(line.size());
    for (size_t i = 0; i < line.size(); ++i) {
        if (line[i] == '\x1b') {
            ++i;
            if (i < line.size() && line[i] == '[') {
                ++i;
                while (i < line.size() && !((line[i] >= 'A' && line[i] <= 'Z') || (line[i] >= 'a' && line[i] <= 'z'))) {
                    ++i;
                }
            }
        } else {
            result += line[i];
        }
    }
    return result;
}

/**
 * @brief Calculate the visible width of a line, excluding ANSI escape codes.
 * This method counts the number of visible characters in the input string, ignoring any ANSI escape codes that may be present. It correctly handles multi-byte UTF-8 characters by counting only the leading bytes of each character.
 * @param line The input string that may contain ANSI escape codes.
 * @return The visible width of the line, excluding ANSI escape codes.
 */
int ObjectRenderer::visibleWidth(const std::string &line) const
{
    std::string clean = stripAnsi(line);
    int count = 0;
    for (size_t i = 0; i < clean.size(); ++i) {
        if ((clean[i] & 0xC0) != 0x80) {
            ++count;
        }
    }
    return count;
}

/**
 * @brief Calculate the horizontal anchor point based on the current alignment.
 * This method determines the horizontal anchor point (in terms of character width) for the object based on its alignment setting. The anchor point is used to position the object correctly when rendering, especially when the alignment is set to center or right.
 * @return The horizontal anchor point in character width.
 */
int ObjectRenderer::anchorX() const
{
    switch (align_) {
        case Align::Center:
        case Align::TopCenter:
        case Align::MiddleCenter:
        case Align::BottomCenter:
            return maxWidth_ / 2;
        case Align::Right:
        case Align::TopRight:
        case Align::MiddleRight:
        case Align::BottomRight:
            return maxWidth_;
        default:
            return 0;
    }
}

/**
 * @brief Calculate the vertical anchor point based on the current alignment.
 * This method determines the vertical anchor point (in terms of character height) for the object based on its alignment setting. The anchor point is used to position the object correctly when rendering, especially when the alignment is set to middle or bottom.
 * @return The vertical anchor point in character height.
 */
int ObjectRenderer::anchorY() const
{
    int h = (int)lines_.size();
    switch (align_) {
        case Align::MiddleLeft:
        case Align::MiddleCenter:
        case Align::MiddleRight:
            return h / 2;
        case Align::BottomLeft:
        case Align::BottomCenter:
        case Align::BottomRight:
            return h;
        default:
            return 0;
    }
}

/**
 * @brief Convert the object renderer to a string representation.
 * This method generates a string that visually represents the object, including its content and alignment. The output can be printed to the console to display the object. It calculates the necessary padding based on the alignment and displacement settings, and constructs the final string with appropriate spacing and newlines.
 * @return A string representation of the object renderer.
 */
std::string ObjectRenderer::toString() const
{
    if (lines_.empty()) return "";

    // When a container (like Partition) handles positioning, output raw lines
    if (externalPos_) {
        std::ostringstream ss;
        for (const auto &line : lines_) {
            ss << line << '\n';
        }
        return ss.str();
    }

    int consoleW = ComponentBase::getConsoleWidth();
    int consoleH = ComponentBase::getConsoleHeight();
    int ax = anchorX();
    int ay = anchorY();

    auto horzTarget = [&]() -> int {
        switch (align_) {
            case Align::Center:
            case Align::TopCenter:
            case Align::MiddleCenter:
            case Align::BottomCenter:
                return consoleW / 2;
            case Align::Right:
            case Align::TopRight:
            case Align::MiddleRight:
            case Align::BottomRight:
                return consoleW;
            default:
                return 0;
        }
    };
    auto vertTarget = [&]() -> int {
        switch (align_) {
            case Align::MiddleLeft:
            case Align::MiddleCenter:
            case Align::MiddleRight:
                return consoleH / 2;
            case Align::BottomLeft:
            case Align::BottomCenter:
            case Align::BottomRight:
                return consoleH;
            default:
                return 0;
        }
    };

    int padLeft = horzTarget() - ax;
    int padTop  = vertTarget() - ay;

    int dirX = 1;
    switch (align_) {
        case Align::Right: case Align::TopRight:
        case Align::MiddleRight: case Align::BottomRight:
            dirX = -1; break;
        default: break;
    }
    int dirY = 1;
    switch (align_) {
        case Align::BottomLeft: case Align::BottomCenter:
        case Align::BottomRight:
            dirY = -1; break;
        default: break;
    }
    padLeft += (int)(displacementX_ * maxWidth_  * dirX / 100.0f);
    padTop  += (int)(displacementY_ * (int)lines_.size() * dirY / 100.0f);

    if (padLeft < 0) padLeft = 0;
    if (padTop < 0) padTop = 0;

    std::ostringstream ss;
    for (int i = 0; i < padTop; ++i) {
        ss << '\n';
    }
    for (const auto &line : lines_) {
        for (int i = 0; i < padLeft; ++i) ss << ' ';
        ss << line << '\n';
    }
    return ss.str();
}

} // namespace ui
