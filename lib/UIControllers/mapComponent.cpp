#include "mapComponent.h"
#include <iostream>
#include <sstream>
#include <algorithm>

#if defined(_WIN32) || defined(_WIN64)
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
    #include <conio.h>
    #ifdef max
        #undef max
    #endif
    #ifdef min
        #undef min
    #endif
#elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    #include <sys/select.h>
    #include <sys/ioctl.h>
    #include <termios.h>
    #include <unistd.h>
    #include <fcntl.h>
#endif

namespace ui {

/**
 * @brief Get the console width in columns.
 * This function retrieves the current width of the console window in terms of columns. It uses platform-specific APIs to query the console dimensions. If the console width cannot be determined, it returns a default value of 80 columns.
 * @return The width of the console in columns.
 */
static int getConsoleWidth()
{
#if defined(_WIN32) || defined(_WIN64)
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return csbi.srWindow.Right - csbi.srWindow.Left + 1;
    }
#elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        return w.ws_col;
    }
#endif
    return 80;
}

/**
 * @brief Get the console height in rows.
 * This function retrieves the current height of the console window in terms of rows. It uses platform-specific APIs to query the console dimensions. If the console height cannot be determined, it returns a default value of 25 rows.
 * @return The height of the console in rows.
 */
static int getConsoleHeight()
{
#if defined(_WIN32) || defined(_WIN64)
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    }
#elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        return w.ws_row;
    }
#endif
    return 25;
}


MapComponent::MapComponent(int framesPerSecond, int gridColumns)
    : callbacks_(),
      cells_(),
      fps_(framesPerSecond > 0 ? framesPerSecond : 30),
      running_(false),
      lastFrame_(),
      focusRow_(0),
      focusCol_(0),
      hasFocus_(false),
      firstFrame_(true),
      lastConsoleWidth_(getConsoleWidth()),
      gridColumns_(gridColumns > 0 ? gridColumns : 3),
      attachedCount_(0),
      console_()
{
}

/**
 * @brief Add a render callback to the MapComponent.
 * Registers a new render callback function that will be called during the rendering process. The callback should accept a reference to an output stream where it can write its rendered content.
 * @param callback A function or lambda that takes an `std::ostream&` parameter and renders content to it.
 * ## Example
 * ```cpp
 * ui::MapComponent map;
 * map.add([](std::ostream &out) {
 *   out << "Rendering callback content" << std::endl;
 * });
 * ```
 */
void MapComponent::add(const RenderCallback &callback)
{
    callbacks_.push_back(callback);
}

/**
 * @brief Place a component on the map.
 * Adds a new component to the map at the specified grid position.
 * @param row The row position where the component will be placed.
 * @param col The column position where the component will be placed.
 * @param component A pointer to the component to be placed on the map.
 */
void MapComponent::place(int row, int col, IComponent *component)
{
    cells_.push_back({component, row, col});
}

/**
 * @brief Attach a component to the map based on its alignment.
 * This method attaches a component to the map by determining its column position based on its alignment. It calculates the appropriate row for the component in that column and adds it to the internal list of cells. The method also increments the count of attached components.
 * @param component A pointer to the component to be attached to the map.
 */
void MapComponent::attach(IComponent *component)
{
    Align a = component->alignment();
    int col;
    switch (a) {
        case Align::Center:
        case Align::TopCenter:
        case Align::MiddleCenter:
        case Align::BottomCenter:
            col = 1;
            break;
        case Align::Right:
        case Align::TopRight:
        case Align::MiddleRight:
        case Align::BottomRight:
            col = 2;
            break;
        default:
            col = 0;
            break;
    }
    int row = 0;
    for (auto &cell : cells_) {
        if (cell.col == col) ++row;
    }
    cells_.push_back({component, row, col});
    ++attachedCount_;
}

/**
 * @brief Find the focusable cell in a given direction.
 * Searches for the closest focusable cell in the specified direction from a starting position.
 * @param fromRow The row position to start the search from.
 * @param fromCol The column position to start the search from.
 * @param dRow The row direction to search (negative for up, positive for down).
 * @param dCol The column direction to search (negative for left, positive for right).
 * @return A pointer to the found focusable cell, or nullptr if none is found.
 */
MapComponent::GridCell *MapComponent::findFocusable(int fromRow, int fromCol, int dRow, int dCol)
{
    // Strict pass: require same row (horizontal) or same column (vertical)
    auto strict = [&]() -> GridCell* {
        int bestRow = -1, bestCol = -1;
        GridCell *best = nullptr;
        for (auto &cell : cells_) {
            if (!cell.component || !cell.component->isFocusable()) continue;
            if (dRow != 0 && cell.col != fromCol) continue;
            if (dCol != 0 && cell.row != fromRow) continue;
            if (dRow > 0 && cell.row <= fromRow) continue;
            if (dRow < 0 && cell.row >= fromRow) continue;
            if (dCol > 0 && cell.col <= fromCol) continue;
            if (dCol < 0 && cell.col >= fromCol) continue;
            int dist = std::abs(cell.row - fromRow) + std::abs(cell.col - fromCol);
            int bestDist = std::abs(bestRow - fromRow) + std::abs(bestCol - fromCol);
            if (!best || dist < bestDist) {
                best = &cell;
                bestRow = cell.row;
                bestCol = cell.col;
            }
        }
        return best;
    };

    GridCell *result = strict();
    if (result) return result;

    // Relaxed pass: any cell in the given direction (allows cross-row
    // horizontal and cross-column vertical navigation)
    {
        int bestRow = -1, bestCol = -1;
        GridCell *best = nullptr;
        for (auto &cell : cells_) {
            if (!cell.component || !cell.component->isFocusable()) continue;
            if (dRow > 0 && cell.row <= fromRow) continue;
            if (dRow < 0 && cell.row >= fromRow) continue;
            if (dCol > 0 && cell.col <= fromCol) continue;
            if (dCol < 0 && cell.col >= fromCol) continue;
            if (dRow == 0 && dCol == 0) continue;
            int dist = std::abs(cell.row - fromRow) + std::abs(cell.col - fromCol);
            int bestDist = std::abs(bestRow - fromRow) + std::abs(bestCol - fromCol);
            if (!best || dist < bestDist) {
                best = &cell;
                bestRow = cell.row;
                bestCol = cell.col;
            }
        }
        if (best) return best;
    }

    // Any-direction fallback: closest focusable cell regardless of direction
    // Handles the case where all components share the same column
    // (same alignment) so left/right can still cycle through them.
    {
        int bestRow = -1, bestCol = -1;
        GridCell *best = nullptr;
        for (auto &cell : cells_) {
            if (!cell.component || !cell.component->isFocusable()) continue;
            if (cell.row == fromRow && cell.col == fromCol) continue;
            int dist = std::abs(cell.row - fromRow) + std::abs(cell.col - fromCol);
            int bestDist = std::abs(bestRow - fromRow) + std::abs(bestCol - fromCol);
            if (!best || dist < bestDist) {
                best = &cell;
                bestRow = cell.row;
                bestCol = cell.col;
            }
        }
        return best;
    }
}

/**
 * @brief Move the focus up.
 * Moves the focus to the closest focusable cell above the current position.
 * @return true if the focus was moved, false otherwise.
 */
bool MapComponent::moveUp()
{
    if (!hasFocus_) return false;
    GridCell *next = findFocusable(focusRow_, focusCol_, -1, 0);
    if (!next) return false;

    for (auto &cell : cells_) {
        if (cell.component) cell.component->setSelected(false);
    }
    focusRow_ = next->row;
    focusCol_ = next->col;
    next->component->setSelected(true);
    return true;
}

/**
 * @brief Move the focus down.
 * Moves the focus to the closest focusable cell below the current position.
 * @return true if the focus was moved, false otherwise.
 */
bool MapComponent::moveDown()
{
    if (!hasFocus_) return false;
    GridCell *next = findFocusable(focusRow_, focusCol_, 1, 0);
    if (!next) return false;

    for (auto &cell : cells_) {
        if (cell.component) cell.component->setSelected(false);
    }
    focusRow_ = next->row;
    focusCol_ = next->col;
    next->component->setSelected(true);
    return true;
}

/**
 * @brief Move the focus left.
 * Moves the focus to the closest focusable cell to the left of the current position.
 * @return true if the focus was moved, false otherwise.
 */
bool MapComponent::moveLeft()
{
    if (!hasFocus_) return false;
    int n = (int)cells_.size();
    int idx = -1;
    for (int i = 0; i < n; ++i) {
        if (cells_[i].row == focusRow_ && cells_[i].col == focusCol_) {
            idx = i;
            break;
        }
    }
    if (idx < 0) return false;
    for (int i = 1; i <= n; ++i) {
        int prev = (idx - i + n) % n;
        if (cells_[prev].component && cells_[prev].component->isFocusable()) {
            for (auto &cell : cells_) cell.component->setSelected(false);
            focusRow_ = cells_[prev].row;
            focusCol_ = cells_[prev].col;
            cells_[prev].component->setSelected(true);
            return true;
        }
    }
    return false;
}

/**
 * @brief Move the focus right.
 * Moves the focus to the closest focusable cell to the right of the current position.
 * @return true if the focus was moved, false otherwise.
 */
bool MapComponent::moveRight()
{
    if (!hasFocus_) return false;
    int n = (int)cells_.size();
    int idx = -1;
    for (int i = 0; i < n; ++i) {
        if (cells_[i].row == focusRow_ && cells_[i].col == focusCol_) {
            idx = i;
            break;
        }
    }
    if (idx < 0) return false;
    for (int i = 1; i <= n; ++i) {
        int next = (idx + i) % n;
        if (cells_[next].component && cells_[next].component->isFocusable()) {
            for (auto &cell : cells_) cell.component->setSelected(false);
            focusRow_ = cells_[next].row;
            focusCol_ = cells_[next].col;
            cells_[next].component->setSelected(true);
            return true;
        }
    }
    return false;
}

/**
 * @brief Activate the currently focused component.
 * Triggers the activation event for the component that currently has focus.
 * @return true if the component was activated, false otherwise.
 */
bool MapComponent::activate()
{
    if (!hasFocus_) return false;
    for (auto &cell : cells_) {
        if (cell.component && cell.row == focusRow_ && cell.col == focusCol_) {
            cell.component->onActivate();
            return true;
        }
    }
    return false;
}

/**
 * @brief Focus the first focusable component.
 * Sets the focus to the first focusable component in the map.
 */
void MapComponent::focusFirstFocusable()
{
    for (auto &cell : cells_) {
        if (cell.component && cell.component->isFocusable()) {
            focusRow_ = cell.row;
            focusCol_ = cell.col;
            hasFocus_ = true;
            cell.component->setSelected(true);
            return;
        }
    }
    hasFocus_ = false;
}

/**
 * @brief Render the map component.
 * Draws the map component and its child components.
 * @param out The output stream to render to.
 */
void MapComponent::render(std::ostream &out)
{
    struct Rect { int row, col, width, height; };
    std::vector<Rect> occupied;

    int consoleWidth = getConsoleWidth();
    int consoleHeight = getConsoleHeight();

    // --- Render callbacks and count their rows ---
    std::ostringstream cbBuf;
    for (const auto &cb : callbacks_) {
        cb(cbBuf);
    }
    std::string cbStr = cbBuf.str();
    int cbRows = 0;
    for (char c : cbStr) {
        if (c == '\n') ++cbRows;
    }
    out << cbStr;

    // --- Render cells with overlap avoidance ---
    for (auto &cell : cells_) {
        if (!cell.component) continue;

        int compWidth = cell.component->width();
        int compHeight = cell.component->height();
        Align align = cell.component->alignment();

        int col = cell.col;
        int row = cell.row;

        bool usesGridRow = true;
        switch (align) {
            case Align::Left:
            case Align::TopLeft:
                break;
            case Align::Center:
            case Align::TopCenter:
                col = (consoleWidth - compWidth) / 2;
                break;
            case Align::Right:
            case Align::TopRight:
                col = consoleWidth - compWidth;
                break;
            case Align::MiddleLeft:
                row = (consoleHeight - compHeight) / 2;
                usesGridRow = false;
                break;
            case Align::MiddleCenter:
                col = (consoleWidth - compWidth) / 2;
                row = (consoleHeight - compHeight) / 2;
                usesGridRow = false;
                break;
            case Align::MiddleRight:
                col = consoleWidth - compWidth;
                row = (consoleHeight - compHeight) / 2;
                usesGridRow = false;
                break;
            case Align::BottomLeft:
                row = consoleHeight - compHeight;
                usesGridRow = false;
                break;
            case Align::BottomCenter:
                col = (consoleWidth - compWidth) / 2;
                row = consoleHeight - compHeight;
                usesGridRow = false;
                break;
            case Align::BottomRight:
                col = consoleWidth - compWidth;
                row = consoleHeight - compHeight;
                usesGridRow = false;
                break;
        }

        // Offset below callbacks for top-aligned (grid-row-based) components
        if (usesGridRow) {
            row += cbRows;
        }

        // Apply percentage displacement based on alignment direction
        {
            float dx = cell.component->displacementX();
            float dy = cell.component->displacementY();
            int dirX = 1;
            switch (align) {
                case Align::Right: case Align::TopRight:
                case Align::MiddleRight: case Align::BottomRight:
                    dirX = -1; break;
                default: break;
            }
            int dirY = 1;
            switch (align) {
                case Align::BottomLeft: case Align::BottomCenter:
                case Align::BottomRight:
                    dirY = -1; break;
                default: break;
            }
            col += (int)(dx * compWidth  * dirX / 100.0f);
            row += (int)(dy * compHeight * dirY / 100.0f);
        }

        if (col < 0) col = 0;
        if (row < 0) row = 0;

        // --- Resolve overlap with other cells by shifting horizontally ---
        for (int attempt = 0; attempt < 20; ++attempt) {
            bool conflict = false;
            for (auto &o : occupied) {
                if (row < o.row + o.height && row + compHeight > o.row &&
                    col < o.col + o.width && col + compWidth > o.col) {
                    int rightEdge = o.col + o.width;
                    if (rightEdge + compWidth <= consoleWidth) {
                        col = rightEdge;
                    } else {
                        int leftEdge = o.col - compWidth;
                        if (leftEdge >= 0) {
                            col = leftEdge;
                        }
                    }
                    conflict = true;
                    break;
                }
            }
            if (!conflict) break;
        }

        // --- Render line by line with cursor positioning ---
        std::string content = cell.component->toString();
        size_t pos = 0;
        int lineNum = 0;
        while (pos < content.size()) {
            size_t next = content.find('\n', pos);
            std::string line = (next == std::string::npos)
                ? content.substr(pos)
                : content.substr(pos, next - pos);
            out << "\x1b[" << (row + lineNum + 1) << ";" << (col + 1) << "H";
            out << line;
            ++lineNum;
            if (next == std::string::npos) break;
            pos = next + 1;
        }

        occupied.push_back({row, col, compWidth, compHeight});
    }
}

/**
 * @brief Handle input events.
 * Processes input events and triggers appropriate actions.
 * @return true if an input event was handled, false otherwise.
 */


bool MapComponent::handleInput()
{

#if defined(_WIN32) || defined(_WIN64)
    if (!_kbhit()) return false;

    int ch = _getch();
    if (ch == 0xE0 || ch == 0x00) {
        ch = _getch();
        switch (ch) {
            case UP_ARROW: moveUp(); return true;
            case DOWN_ARROW: moveDown(); return true;
            case LEFT_ARROW: moveLeft(); return true;
            case RIGHT_ARROW: moveRight(); return true;
        }
    } else {
        switch (ch) {
            case 13: activate(); return true;
            case 27: running_ = false; return true;
            case '\t':
                if (moveDown()) return true;
                focusFirstFocusable();
                return true;
        }
    }
#elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    struct timeval tv = {0, 0};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);

    bool handled = false;
    if (select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv) > 0) {
        char ch;
        if (read(STDIN_FILENO, &ch, 1) > 0) {
            if (ch == '\x1b') {
                char seq[2];
                if (read(STDIN_FILENO, &seq[0], 1) > 0 && seq[0] == '[') {
                    if (read(STDIN_FILENO, &seq[1], 1) > 0) {
                        switch (seq[1]) {
                            case 'A': moveUp(); handled = true; break;
                            case 'B': moveDown(); handled = true; break;
                            case 'D': moveLeft(); handled = true; break;
                            case 'C': moveRight(); handled = true; break;
                        }
                    }
                }
            } else {
                switch (ch) {
                    case '\n': case '\r': activate(); handled = true; break;
                    case 27: running_ = false; handled = true; break;
                    case '\t':
                        if (!moveDown()) focusFirstFocusable();
                        handled = true;
                        break;
                }
            }
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return handled;
#endif
    return false;
}

/**
 * @brief Run the map component.
 * Starts the main loop for the map component.
 */
void MapComponent::run()
{
#if defined(_WIN32) || defined(_WIN64)
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        if (GetConsoleMode(hOut, &mode)) {
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, mode);
        }
    }

    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hOut, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hOut, &cursorInfo);
#endif

    focusFirstFocusable();
    running_ = true;

    if (!background_.empty()) {
        std::cout << "\x1b]11;" << background_ << "\x07";
        std::cout.flush();
    }

    const auto frameDelay = std::chrono::milliseconds(1000 / fps_);

    while (running_) {
        std::ostringstream buffer;
        render(buffer);
        const std::string frame = buffer.str();

        if (frame != lastFrame_) {
            int curWidth = getConsoleWidth();
            bool resized = curWidth != lastConsoleWidth_;
            if (firstFrame_ || resized) {
                std::cout << "\x1b[2J\x1b[H";
                lastConsoleWidth_ = curWidth;
                firstFrame_ = false;
            } else {
                std::cout << "\x1b[H";
            }
            std::cout << frame;
            std::cout.flush();
            lastFrame_ = frame;
        }

        handleInput();

        if (running_ && frameDelay.count() > 0) {
            std::this_thread::sleep_for(frameDelay);
        }
    }
}

/**
 * @brief Stop the map component.
 * Signals the main loop to stop running and exit.
 */
void MapComponent::stop()
{
    running_ = false;
    std::cout << "\x1b]11;#000000\x07";
    std::cout << "\x1b[0m";
    std::cout.flush();
}

} // namespace ui
