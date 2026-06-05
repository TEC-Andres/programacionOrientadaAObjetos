#include "consoleInstance.h"

// Parametrized constructor
ConsoleInstance::ConsoleInstance(int width, int height) {
    #if defined(_WIN32) || defined(_WIN64)
        system("cls");
    #elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
        system("clear");
    #endif
    ConsoleInstance::applyConsoleSize(width, height);
    ConsoleInstance::anchorConsole(AnchorPosition::TopLeft);
    ConsoleInstance::removeScrollbar();
}

// Default constructor
ConsoleInstance::ConsoleInstance() {
    #if defined(_WIN32) || defined(_WIN64)
        system("cls");
    #elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
        system("clear");
    #endif
    int screenWidth = 0, screenHeight = 0;
    ConsoleInstance::getScreenSize(screenWidth, screenHeight);
    if (screenWidth > 0 && screenHeight > 0) {
        ConsoleInstance::applyConsoleSize(screenWidth, screenHeight);
    }
    ConsoleInstance::anchorConsole(AnchorPosition::TopLeft);
    ConsoleInstance::removeScrollbar();
}

ConsoleInstance::~ConsoleInstance() {
    // Destructor logic if needed
}

/**
 * @brief Get the console window size.
 *
 * Retrieves the current console window size and updates the provided
 * `width` and `height` output parameters. The measured size is also stored
 * in the internal `consoleSize` vector for later access.
 *
 * @param[out] width  Receives the console width (columns).
 * @param[out] height Receives the console height (rows).
 * @return A `std::vector<int>` containing {width, height}.
 *
 * ## Example
 * ```cpp
 * int width, height;
 * ConsoleInstance console;
 * console.getConsoleSize(width, height);
 * ```
 */
std::vector<int> ConsoleInstance::getConsoleSize(int& width, int& height) {
    width = 0;
    height = 0;
    #if defined(_WIN32) || defined(_WIN64)
        CONSOLE_SCREEN_BUFFER_INFO scrBufferInfo;
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &scrBufferInfo)) {
            width = scrBufferInfo.srWindow.Right - scrBufferInfo.srWindow.Left + 1;
            height = scrBufferInfo.srWindow.Bottom - scrBufferInfo.srWindow.Top + 1;
        }
    #elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
        struct winsize w;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
            width = w.ws_col;
            height = w.ws_row;
        }
    #endif
    consoleSize.clear();
    consoleSize.push_back(width);
    consoleSize.push_back(height);
    return consoleSize;
}

/**
 * @brief Set the console window size.
 * 
 * Adjusts the console window to the specified width and height. The method
 * uses platform-specific APIs to resize the console window and updates the
 * internal `consoleSize` vector to reflect the new dimensions.
 * @param width  The desired console width (columns).
 * @param height The desired console height (rows).
 * 
 * ## Example
 * ```cpp
 * ConsoleInstance console;
 * console.setConsoleSize(100, 30);
 * ```
 */
void ConsoleInstance::setConsoleSize(int width, int height) {
    #if defined(_WIN32) || defined(_WIN64)
        SMALL_RECT windowSize = {0, 0, static_cast<SHORT>(width - 1), static_cast<SHORT>(height - 1)};
        SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE), TRUE, &windowSize);
    #elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
        struct winsize w;
        w.ws_col = width;
        w.ws_row = height;
        ioctl(STDOUT_FILENO, TIOCSWINSZ, &w);
    #endif
    consoleSize.clear();
    consoleSize.push_back(width);
    consoleSize.push_back(height);
}

/**
 * @brief Show or hide the console cursor.
 * 
 * Toggles the visibility of the console cursor based on the `show` parameter.
 * The implementation uses platform-specific APIs to achieve this effect.
 * 
 * @param show If `true`, the cursor will be shown; if `false`, it will be hidden.
 * 
 * ## Example
 * ```cpp
 * ConsoleInstance console;
 * console.showCursor(false); // Hides the cursor
 * console.showCursor(true);  // Shows the cursor
 * ```
 */
void ConsoleInstance::showCursor(bool show) {
    #if defined(_WIN32) || defined(_WIN64)
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
        cursorInfo.bVisible = show ? TRUE : FALSE;
        SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
    #elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
        std::cout << (show ? "\x1b[?25h" : "\x1b[?25l");
        std::cout.flush();
    #endif
}


/**
 * @brief Get the current cursor position in the console.
 * 
 * Retrieves the current position of the console cursor and updates the provided
 * `x` and `y` output parameters with the cursor's column and row, respectively.
 * The method uses platform-specific APIs to obtain the cursor position.
 * @param[out] x Receives the cursor's column position (0-based).
 * @param[out] y Receives the cursor's row position (0-based).
 * @return An integer status code (0 for success, non-zero for failure).
 * 
 * ## Example
 * ```cpp
 * int x, y;
 * ConsoleInstance console;
 * if (console.getCursorPosition(x, y) == 0) {
 *   std::cout << "Cursor Position: (" << x << ", " << y << ")" << std::endl;   
 * }
 * ```
 */
void ConsoleInstance::getScreenSize(int& width, int& height) {
    width = 0;
    height = 0;
    #if defined(_WIN32) || defined(_WIN64)
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut != INVALID_HANDLE_VALUE) {
            COORD largest = GetLargestConsoleWindowSize(hOut);
            if (largest.X > 0 && largest.Y > 0) {
                width = static_cast<int>(largest.X);
                height = static_cast<int>(largest.Y);
            }
        }
    #elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
        struct winsize w;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
            width = w.ws_col;
            height = w.ws_row;
        }
    #endif
}

/**
 * @brief Remove the console scrollbar.
 * 
 * This method attempts to remove the scrollbar from the console window by resizing
 * the console buffer to match the window size. The implementation uses platform-specific
 * APIs to achieve this effect. Note that on Unix-like systems, true scrollbar removal is 
 * not possible, but resizing the console can effectively hide it.
 * 
 * ## Example
 * ```cpp
 * ConsoleInstance console;
 * console.removeScrollbar();
 * ```
 */
void ConsoleInstance::removeScrollbar() {
    #if defined(_WIN32) || defined(_WIN64)
        CONSOLE_SCREEN_BUFFER_INFO scrBufferInfo;
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &scrBufferInfo)) {
            COORD newSize = { static_cast<SHORT>(consoleSize[0]), static_cast<SHORT>(consoleSize[1]) };
            SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), newSize);
        }
    #elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
        /**
         * There's no UNIX-like method that allows you to remove the scrollbar
         * but we can hardcode the console size to match.
         */

        struct winsize w;
        w.ws_col = consoleSize[0];
        w.ws_row = consoleSize[1];
        ioctl(STDOUT_FILENO, TIOCSWINSZ, &w);
    #endif
}


// void ConsoleInstance::setBackgroundColor() {
//
// }

/**
 * @brief Apply the specified console size.
 * 
 * Resizes the console window to the given width and height. The method uses platform-specific
 * APIs to adjust the console size and updates the internal `consoleSize` vector accordingly.
 * @param width  The desired console width (columns).
 * @param height The desired console height (rows).
 * 
 * ## Example
 * ```cpp
 * ConsoleInstance console;
 * console.applyConsoleSize(80, 25);
 * ```
 */
void ConsoleInstance::applyConsoleSize(int width, int height) {
    if (width <= 0 || height <= 0) return;
#if defined(_WIN32) || defined(_WIN64)
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        // Ensure buffer is at least window size, then set window size
        COORD newBuf = { static_cast<SHORT>(width), static_cast<SHORT>(height) };
        SetConsoleScreenBufferSize(hOut, newBuf);
        SMALL_RECT windowSize = {0, 0, static_cast<SHORT>(width - 1), static_cast<SHORT>(height - 1)};
        SetConsoleWindowInfo(hOut, TRUE, &windowSize);
        // Re-anchor to top-left after resizing
        HWND hWnd = GetConsoleWindow();
        if (hWnd) {
            SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        }
    }
#elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    struct winsize w;
    w.ws_col = width;
    w.ws_row = height;
    ioctl(STDOUT_FILENO, TIOCSWINSZ, &w);
#endif
    consoleSize.clear();
    consoleSize.push_back(width);
    consoleSize.push_back(height);
}

/**
 * @brief Anchor the console window to the specified corner of the screen.
 * 
 * This method attempts to anchor the console window to the specified corner of the screen
 * using platform-specific APIs. On Unix-like systems, this operation may not be applicable
 * as terminal emulators typically manage their own window positioning.
 * 
 * ## Example
 * ```cpp
 * ConsoleInstance console;
 * console.anchorConsole(ConsoleInstance::AnchorPosition::TopLeft);
 * ```
 */
void ConsoleInstance::anchorConsole(AnchorPosition position) {
    #if defined(_WIN32) || defined(_WIN64)
    HWND hWnd = GetConsoleWindow();
    if (!hWnd) return;
    RECT desktopRect, consoleRect;
    GetWindowRect(GetDesktopWindow(), &desktopRect);
    GetWindowRect(hWnd, &consoleRect);

    int winW = consoleRect.right - consoleRect.left;
    int winH = consoleRect.bottom - consoleRect.top;
    int screenW = desktopRect.right;
    int screenH = desktopRect.bottom;

    int x = 0;
    int y = 0;
    switch (position) {
        case AnchorPosition::TopRight:
            x = screenW - winW;
            break;
        case AnchorPosition::BottomLeft:
            y = screenH - winH;
            break;
        case AnchorPosition::BottomRight:
            x = screenW - winW;
            y = screenH - winH;
            break;
        case AnchorPosition::TopLeft:
        default:
            break;
    }

    SetWindowPos(hWnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

    #elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    (void)position; 
    #endif
}