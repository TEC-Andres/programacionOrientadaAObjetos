#include "consoleInstance.h"

ConsoleInstance::ConsoleInstance(int width, int height) {
    initializeConsole();
    ConsoleInstance::applyConsoleSize(width, height);
    ConsoleInstance::anchorConsoleTopLeft();
    ConsoleInstance::removeScrollbar();

}

ConsoleInstance::ConsoleInstance() {
    initializeConsole();
    int screenWidth = 0, screenHeight = 0;
    ConsoleInstance::getScreenSize(screenWidth, screenHeight);
    if (screenWidth > 0 && screenHeight > 0) {
        ConsoleInstance::applyConsoleSize(screenWidth, screenHeight);
    }
    ConsoleInstance::anchorConsoleTopLeft();
    ConsoleInstance::removeScrollbar();
}

ConsoleInstance::~ConsoleInstance() {
    // Destructor logic if needed
}

void ConsoleInstance::initializeConsole() {
    #if defined(_WIN32) || defined(_WIN64)
        system("cls");
    #elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
        system("clear");
    #endif
}

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

void ConsoleInstance::removeScrollbar() {
    #if defined(_WIN32) || defined(_WIN64)
        CONSOLE_SCREEN_BUFFER_INFO scrBufferInfo;
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &scrBufferInfo)) {
            COORD newSize = { static_cast<SHORT>(consoleSize[0]), static_cast<SHORT>(consoleSize[1]) };
            SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), newSize);
        }
    #elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
        // No hay metodo para remover scrollbar en terminales Unix-like, pero ajustar el tamaño del terminal puede ayudar a evitarlo
        struct winsize w;
        w.ws_col = consoleSize[0];
        w.ws_row = consoleSize[1];
        ioctl(STDOUT_FILENO, TIOCSWINSZ, &w);
    #endif
}

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

void ConsoleInstance::applyConsoleSize(int width, int height) {
    if (width <= 0 || height <= 0) return;
#if defined(_WIN32) || defined(_WIN64)
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        // Asegurar que el buffer sea al menos tan grande como la ventana para evitar errores
        COORD newBuf = { static_cast<SHORT>(width), static_cast<SHORT>(height) };
        SetConsoleScreenBufferSize(hOut, newBuf);
        SMALL_RECT windowSize = {0, 0, static_cast<SHORT>(width - 1), static_cast<SHORT>(height - 1)};
        SetConsoleWindowInfo(hOut, TRUE, &windowSize);
        // Hacer que la ventana se ancle a la esquina superior izquierda después de aplicar el tamaño
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

void ConsoleInstance::anchorConsoleTopLeft() {
    #if defined(_WIN32) || defined(_WIN64)
    HWND hWnd = GetConsoleWindow();
    if (hWnd) {
        SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }
    #elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    // TODO & TO TEST: Implement anchoring for Unix-like systems if needed, as it may require terminal-specific escape codes or settings
     // Note: Anchoring to top-left is typically not applicable in Unix-like terminal emulators, as they manage their own window positioning.
    #endif
}