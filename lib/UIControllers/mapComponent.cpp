#include "mapComponent.h"
#include "partition.h"
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
    #include <signal.h>
#endif

namespace ui {

#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
static volatile sig_atomic_t g_resize_pending_ = 0;
extern "C" void handle_winch(int) { g_resize_pending_ = 1; }
#endif

static int alignmentBand(Align a) {
    switch (a) {
        case Align::Left: case Align::Center: case Align::Right:
        case Align::TopLeft: case Align::TopCenter: case Align::TopRight:
            return 0;
        case Align::MiddleLeft: case Align::MiddleCenter: case Align::MiddleRight:
            return 1;
        case Align::BottomLeft: case Align::BottomCenter: case Align::BottomRight:
            return 2;
    }
    return 0;
}

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


MapComponent::MapComponent(int gridColumns)
    : callbacks_(),
      cells_(),
      running_(false),
      needsRedraw_(false),
      focusRow_(0),
      focusCol_(0),
      hasFocus_(false),
      consoleWidth_(0),
      consoleHeight_(0),
      gridColumns_(gridColumns > 0 ? gridColumns : 3),
      attachedCount_(0),
      partition_(nullptr)
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

MapComponent::GridCell *MapComponent::findFocusable(int fromRow, int fromCol, int dRow, int dCol)
{
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

    int band = -1;
    int curIdx = -1;
    for (int i = 0; i < (int)cells_.size(); ++i) {
        auto &cell = cells_[i];
        if (cell.component && cell.row == focusRow_ && cell.col == focusCol_) {
            band = alignmentBand(cell.component->alignment());
            curIdx = i;
            break;
        }
    }
    if (band < 0 || curIdx < 0) return false;

    std::vector<int> sameBand;
    for (int i = 0; i < (int)cells_.size(); ++i) {
        if (cells_[i].component && cells_[i].component->isFocusable() &&
            alignmentBand(cells_[i].component->alignment()) == band) {
            sameBand.push_back(i);
        }
    }
    if (sameBand.size() < 2) {
        GridCell *next = findFocusable(focusRow_, focusCol_, 0, -1);
        if (next) {
            for (auto &c : cells_) if (c.component) c.component->setSelected(false);
            focusRow_ = next->row;
            focusCol_ = next->col;
            next->component->setSelected(true);
            return true;
        }
        return false;
    }

    int pos = -1;
    for (int i = 0; i < (int)sameBand.size(); ++i) {
        if (sameBand[i] == curIdx) { pos = i; break; }
    }
    if (pos < 0) return false;
    int prev = (pos - 1 + (int)sameBand.size()) % (int)sameBand.size();
    int idx = sameBand[prev];

    for (auto &c : cells_) if (c.component) c.component->setSelected(false);
    focusRow_ = cells_[idx].row;
    focusCol_ = cells_[idx].col;
    cells_[idx].component->setSelected(true);
    return true;
}

bool MapComponent::moveRight()
{
    if (!hasFocus_) return false;

    int band = -1;
    int curIdx = -1;
    for (int i = 0; i < (int)cells_.size(); ++i) {
        auto &cell = cells_[i];
        if (cell.component && cell.row == focusRow_ && cell.col == focusCol_) {
            band = alignmentBand(cell.component->alignment());
            curIdx = i;
            break;
        }
    }
    if (band < 0 || curIdx < 0) return false;

    std::vector<int> sameBand;
    for (int i = 0; i < (int)cells_.size(); ++i) {
        if (cells_[i].component && cells_[i].component->isFocusable() &&
            alignmentBand(cells_[i].component->alignment()) == band) {
            sameBand.push_back(i);
        }
    }
    if (sameBand.size() < 2) {
        GridCell *next = findFocusable(focusRow_, focusCol_, 0, 1);
        if (next) {
            for (auto &c : cells_) if (c.component) c.component->setSelected(false);
            focusRow_ = next->row;
            focusCol_ = next->col;
            next->component->setSelected(true);
            return true;
        }
        return false;
    }

    int pos = -1;
    for (int i = 0; i < (int)sameBand.size(); ++i) {
        if (sameBand[i] == curIdx) { pos = i; break; }
    }
    if (pos < 0) return false;
    int next = (pos + 1) % (int)sameBand.size();
    int idx = sameBand[next];

    for (auto &c : cells_) if (c.component) c.component->setSelected(false);
    focusRow_ = cells_[idx].row;
    focusCol_ = cells_[idx].col;
    cells_[idx].component->setSelected(true);
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
    struct Rect { int row, col, width, height; };
    std::vector<Rect> occupied;

    int cw = consoleWidth_;
    int ch = consoleHeight_;

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

    // If a partition is set, delegate rendering to it
    if (partition_) {
        partition_->update(cbRows);
        partition_->render(out);
        // After partition render, clear dirty flags on all components
        for (auto &cell : cells_) {
            if (cell.component) cell.component->clearDirty();
        }
        return;
    }

    // --- Render cells with overlap avoidance ---
    for (auto &cell : cells_) {
        if (!cell.component) continue;

        cell.component->setUsesExternalPositioning(true);

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
                col = (cw - compWidth) / 2;
                break;
            case Align::Right:
            case Align::TopRight:
                col = cw - compWidth;
                break;
            case Align::MiddleLeft:
                row = (ch - compHeight) / 2;
                usesGridRow = false;
                break;
            case Align::MiddleCenter:
                col = (cw - compWidth) / 2;
                row = (ch - compHeight) / 2;
                usesGridRow = false;
                break;
            case Align::MiddleRight:
                col = cw - compWidth;
                row = (ch - compHeight) / 2;
                usesGridRow = false;
                break;
            case Align::BottomLeft:
                row = ch - compHeight;
                usesGridRow = false;
                break;
            case Align::BottomCenter:
                col = (cw - compWidth) / 2;
                row = ch - compHeight;
                usesGridRow = false;
                break;
            case Align::BottomRight:
                col = cw - compWidth;
                row = ch - compHeight;
                usesGridRow = false;
                break;
        }

        if (usesGridRow) {
            row += cbRows;
        }

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

        for (int attempt = 0; attempt < 20; ++attempt) {
            bool conflict = false;
            for (auto &o : occupied) {
                if (row < o.row + o.height && row + compHeight > o.row &&
                    col < o.col + o.width && col + compWidth > o.col) {
                    int rightEdge = o.col + o.width;
                    if (rightEdge + compWidth <= cw) {
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

        // --- Render dirty components only (or all on initial frame) ---
        if (cell.component->isDirty()) {
            std::string content = cell.component->toString();
            size_t pos = 0;
            int lineNum = 0;
            while (pos < content.size()) {
                size_t next = content.find('\n', pos);
                std::string line = (next == std::string::npos)
                    ? content.substr(pos)
                    : content.substr(pos, next - pos);
                out << "\x1b[" << (row + lineNum + 1) << ";" << (col + 1) << "H";
                out << line << "\x1b[K";
                ++lineNum;
                if (next == std::string::npos) break;
                pos = next + 1;
            }
            cell.component->clearDirty();
        }

        occupied.push_back({row, col, compWidth, compHeight});
    }

    // Manage cursor for non-Partition mode
    if (hasFocus_) {
        GridCell *focused = nullptr;
        for (auto &cell : cells_) {
            if (cell.component && cell.row == focusRow_ && cell.col == focusCol_) {
                focused = &cell;
                break;
            }
        }
        if (focused && focused->component && focused->component->wantsCursor()) {
            int curX = focused->component->cursorX();
            int curY = focused->component->cursorY();
            // Find the actual screen position of the component by re-calculating
            int compRow = focused->row, compCol = focused->col;
            Align align = focused->component->alignment();
            switch (align) {
                case Align::Center: case Align::TopCenter:
                    compCol = (consoleWidth_ - focused->component->width()) / 2;
                    break;
                case Align::Right: case Align::TopRight:
                    compCol = consoleWidth_ - focused->component->width();
                    break;
                case Align::MiddleLeft:
                    compRow = (consoleHeight_ - focused->component->height()) / 2;
                    break;
                case Align::MiddleCenter:
                    compCol = (consoleWidth_ - focused->component->width()) / 2;
                    compRow = (consoleHeight_ - focused->component->height()) / 2;
                    break;
                case Align::MiddleRight:
                    compCol = consoleWidth_ - focused->component->width();
                    compRow = (consoleHeight_ - focused->component->height()) / 2;
                    break;
                case Align::BottomLeft:
                    compRow = consoleHeight_ - focused->component->height();
                    break;
                case Align::BottomCenter:
                    compCol = (consoleWidth_ - focused->component->width()) / 2;
                    compRow = consoleHeight_ - focused->component->height();
                    break;
                case Align::BottomRight:
                    compCol = consoleWidth_ - focused->component->width();
                    compRow = consoleHeight_ - focused->component->height();
                    break;
                default: break;
            }
            out << "\x1b[" << (compRow + curY + 1) << ";" << (compCol + curX + 1) << "H";
            out << "\x1b[?25h";
        } else {
            out << "\x1b[?25l";
        }
    } else {
        out << "\x1b[?25l";
    }
}

bool MapComponent::handleInput()
{

#if defined(_WIN32) || defined(_WIN64)
    if (!_kbhit()) return false;

    int ch = _getch();
    if (ch == 0xE0 || ch == 0x00) {
        int raw = _getch();
        int key = raw;
        switch (raw) {
            case 72: key = UP_ARROW; break;
            case 80: key = DOWN_ARROW; break;
            case 75: key = LEFT_ARROW; break;
            case 77: key = RIGHT_ARROW; break;
            case 15: key = SHIFT_TAB; break;
        }
        needsRedraw_ = true;
        if (partition_) {
            if (partition_->handleKey(key)) return true;
        }
        switch (key) {
            case UP_ARROW: moveUp(); return true;
            case DOWN_ARROW: moveDown(); return true;
            case LEFT_ARROW: moveLeft(); return true;
            case RIGHT_ARROW: moveRight(); return true;
        }
    } else {
        needsRedraw_ = true;
        if (partition_) {
            if (partition_->handleKey(ch)) return true;
        }
        if (hasFocus_) {
            for (auto &cell : cells_) {
                if (cell.component && cell.row == focusRow_ && cell.col == focusCol_) {
                    if (cell.component->handleKey(ch)) return true;
                    break;
                }
            }
        }
        switch (ch) {
            case 13: activate(); return true;
            case 27: running_ = false; return true;
            case '\t':
                if (partition_) {
                    partition_->handleKey(9);
                } else {
                    if (!moveDown()) focusFirstFocusable();
                }
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
            needsRedraw_ = true;
            if (ch == '\x1b') {
                char seq[2];
                if (read(STDIN_FILENO, &seq[0], 1) > 0 && seq[0] == '[') {
                    if (read(STDIN_FILENO, &seq[1], 1) > 0) {
                        if (partition_) {
                            int ak = 0;
                            switch (seq[1]) {
                                case 'A': ak = UP_ARROW; break;
                                case 'B': ak = DOWN_ARROW; break;
                                case 'D': ak = LEFT_ARROW; break;
                                case 'C': ak = RIGHT_ARROW; break;
                                case 'Z': ak = SHIFT_TAB; break;
                            }
                            if (ak && partition_->handleKey(ak)) {
                                handled = true;
                            }
                        }
                        if (!handled) {
                            switch (seq[1]) {
                                case 'A': moveUp(); handled = true; break;
                                case 'B': moveDown(); handled = true; break;
                                case 'D': moveLeft(); handled = true; break;
                                case 'C': moveRight(); handled = true; break;
                            }
                        }
                    }
                }
            } else {
                if (partition_ && partition_->handleKey((unsigned char)ch)) {
                    handled = true;
                }
                if (!handled) {
                    for (auto &cell : cells_) {
                        if (cell.component && cell.row == focusRow_ && cell.col == focusCol_) {
                            if (cell.component->handleKey((unsigned char)ch)) {
                                handled = true;
                                break;
                            }
                            break;
                        }
                    }
                }
                if (!handled) {
                    switch (ch) {
                        case '\n': case '\r': activate(); handled = true; break;
                        case 27: running_ = false; handled = true; break;
                        case '\t':
                            if (partition_) {
                                partition_->handleKey(9);
                            } else {
                                if (!moveDown()) focusFirstFocusable();
                            }
                            handled = true;
                            break;
                    }
                }
            }
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return handled;
#endif
    return false;
}

void MapComponent::update()
{
#if defined(_WIN32) || defined(_WIN64)
    if (wakeEvent_) {
        SetEvent((HANDLE)wakeEvent_);
    }
#elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    if (wakePipe_[1] != -1) {
        char c = 1;
        (void)write(wakePipe_[1], &c, 1);
    }
#endif
    needsRedraw_ = true;
}

void MapComponent::clearScreen_()
{
    std::cout << "\x1b[3J\x1b[2J\x1b[H" << std::flush;
}

void MapComponent::updateConsoleSize_()
{
    int cw = getConsoleWidth();
    int ch = getConsoleHeight();
    if (cw != consoleWidth_ || ch != consoleHeight_) {
        consoleWidth_ = cw;
        consoleHeight_ = ch;
        needsRedraw_ = true;
    }
}

void MapComponent::drainInputBuffer_()
{
#if defined(_WIN32) || defined(_WIN64)
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    DWORD nEvents = 0;
    while (true) {
        GetNumberOfConsoleInputEvents(hIn, &nEvents);
        if (nEvents == 0) break;
        INPUT_RECORD ir;
        DWORD read;
        ReadConsoleInput(hIn, &ir, 1, &read);
    }
#elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    fd_set fds;
    struct timeval tv = {0, 0};
    char c;
    while (true) {
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        if (select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) <= 0) break;
        if (read(STDIN_FILENO, &c, 1) <= 0) break;
    }
#endif
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

    // Create wakeup event for update() calls
    wakeEvent_ = CreateEventW(NULL, FALSE, FALSE, NULL);

    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hOut, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hOut, &cursorInfo);
#else
    // Enable raw mode for Linux/Mac
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    // Create wakeup pipe for update() calls
    wakePipe_[0] = -1;
    wakePipe_[1] = -1;
    if (pipe(wakePipe_) == 0) {
        fcntl(wakePipe_[0], F_SETFL, O_NONBLOCK);
        fcntl(wakePipe_[1], F_SETFL, O_NONBLOCK);
    }

    signal(SIGWINCH, handle_winch);
#endif

    // Enable alternate screen buffer
    std::cout << "\033[?1049h";
    // Drain stale events from initialization (e.g. spurious resize)
    drainInputBuffer_();

    focusFirstFocusable();
    if (partition_) {
        partition_->setFocus(true);
    }
    running_ = true;

    if (!background_.empty()) {
        std::cout << "\x1b]11;" << background_ << "\x07";
        std::cout.flush();
    }

    // Initial console size
    consoleWidth_ = getConsoleWidth();
    consoleHeight_ = getConsoleHeight();

    // Set all components dirty for initial full render
    for (auto &cell : cells_) {
        if (cell.component) cell.component->setDirty();
    }

    // Full clear and first frame
    clearScreen_();
    {
        std::ostringstream buf;
        render(buf);
        std::cout << buf.str() << std::flush;
    }

    while (running_) {
#if defined(_WIN32) || defined(_WIN64)
        HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
        HANDLE waitHandles[2] = { hIn, (HANDLE)wakeEvent_ };
        DWORD waitRes = WaitForMultipleObjects(2, waitHandles, FALSE, INFINITE);

        if (waitRes == WAIT_OBJECT_0) {
            while (_kbhit()) {
                handleInput();
            }
        }

        updateConsoleSize_();
#elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        int nfds = STDIN_FILENO + 1;
        if (wakePipe_[0] != -1) {
            FD_SET(wakePipe_[0], &fds);
            if (wakePipe_[0] >= nfds) nfds = wakePipe_[0] + 1;
        }

        struct timeval tv = {0, 50000};
        int selRet = select(nfds, &fds, NULL, NULL, &tv);

        if (selRet > 0) {
            if (wakePipe_[0] != -1 && FD_ISSET(wakePipe_[0], &fds)) {
                char buf[64];
                while (read(wakePipe_[0], buf, sizeof(buf)) > 0) {}
            }

            if (FD_ISSET(STDIN_FILENO, &fds)) {
                handleInput();
            }
        }

        if (g_resize_pending_) {
            g_resize_pending_ = 0;
            auto sz = getConsoleWidth();
            auto sh = getConsoleHeight();
            if (sz != consoleWidth_ || sh != consoleHeight_) {
                consoleWidth_ = sz;
                consoleHeight_ = sh;
                needsRedraw_ = true;
            }
        }
#endif

        if (needsRedraw_) {
            std::ostringstream buf;
            render(buf);
            std::cout << "\x1b[H" << buf.str() << std::flush;
            needsRedraw_ = false;
        }
    }

    // Always show cursor on exit
    std::cout << "\033[?25h";
    std::cout << "\033[?1049l";
    std::cout << "\x1b]11;#000000\x07";
    std::cout << "\x1b[0m";
    std::cout.flush();

#if defined(_WIN32) || defined(_WIN64)
    if (wakeEvent_) {
        CloseHandle((HANDLE)wakeEvent_);
        wakeEvent_ = nullptr;
    }
#else
    if (wakePipe_[0] != -1) { close(wakePipe_[0]); close(wakePipe_[1]); }
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
}

void MapComponent::stop()
{
    running_ = false;
}

} // namespace ui
