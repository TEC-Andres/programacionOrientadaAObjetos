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

void MapComponent::add(const RenderCallback &callback)
{
    callbacks_.push_back(callback);
}

void MapComponent::place(int row, int col, IComponent *component)
{
    cells_.push_back({component, row, col});
}

void MapComponent::clearScreen() const
{
    std::cout << "\x1b[2J\x1b[H";
}

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

bool MapComponent::handleInput()
{
#if defined(_WIN32) || defined(_WIN64)
    if (!_kbhit()) return false;

    int ch = _getch();
    if (ch == 0xE0 || ch == 0x00) {
        ch = _getch();
        switch (ch) {
            case 72: moveUp(); return true;
            case 80: moveDown(); return true;
            case 75: moveLeft(); return true;
            case 77: moveRight(); return true;
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

void MapComponent::stop()
{
    running_ = false;
}

} // namespace ui
