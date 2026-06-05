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
    #include <termios.h>
    #include <unistd.h>
    #include <fcntl.h>
#endif

namespace ui {

/**
 * @brief Construct a new MapComponent object.
 * Initializes the MapComponent with an optional frames-per-second (FPS) setting for rendering. The constructor sets up internal state for managing render callbacks, grid cells, focus, and input handling.
 * @param framesPerSecond The desired frames per second for rendering the map (default: 30). Must be a positive integer; otherwise, it defaults to 30 FPS.
 * ## Example
 * ```cpp
 * ui::MapComponent map(60); // Create a MapComponent with 60 FPS
 * ui::MapComponent defaultMap; // Create a MapComponent with default 30 FPS
 * ```
*/
MapComponent::MapComponent(int framesPerSecond)
    : callbacks_(),
      cells_(),
      fps_(framesPerSecond > 0 ? framesPerSecond : 30),
      running_(false),
      lastFrame_(),
      focusRow_(0),
      focusCol_(0),
      hasFocus_(false)
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
 * @brief Clear the console screen.
 * Uses ANSI escape codes to clear the console screen and reset the cursor position to the top-left corner. This method is called before rendering a new frame to ensure that the previous content is removed.
 */
void MapComponent::clearScreen() const
{
    std::cout << "\x1b[2J\x1b[H";
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
    GridCell *next = findFocusable(focusRow_, focusCol_, 0, -1);
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
 * @brief Move the focus right.
 * Moves the focus to the closest focusable cell to the right of the current position.
 * @return true if the focus was moved, false otherwise.
 */
bool MapComponent::moveRight()
{
    if (!hasFocus_) return false;
    GridCell *next = findFocusable(focusRow_, focusCol_, 0, 1);
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
    for (const auto &cb : callbacks_) {
        cb(out);
    }
    for (auto &cell : cells_) {
        if (cell.component) {
            cell.component->render(out);
        }
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
    const auto frameDelay = std::chrono::milliseconds(1000 / fps_);

    while (running_) {
        std::ostringstream buffer;
        render(buffer);
        const std::string frame = buffer.str();

        if (frame != lastFrame_) {
            clearScreen();
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
}

} // namespace ui
