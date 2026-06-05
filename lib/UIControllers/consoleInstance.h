#pragma once
#include <iostream>
#include <vector>

#if defined(_WIN32) || defined(_WIN64)
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
    #include <wincon.h>
    // Prevent Windows headers from defining macros named min/max which break std::min/std::max
    #ifdef max
        #undef max
    #endif
    #ifdef min
        #undef min
    #endif
#elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    #include <sys/ioctl.h>
    #include <unistd.h>
    #include <stdio.h>
#endif

class ConsoleInstance
{
public:
    ConsoleInstance(int width, int height);
    ConsoleInstance();
    ~ConsoleInstance();

    /* <--- Console size management ---> */ 

    std::vector<int> getConsoleSize(int& width, int& height);
    void setConsoleSize(int width, int height);

    /* <--- Cursor components ---> */ 

    void showCursor(bool show);
    int getCursorPosition(int& x, int& y);

    /* <--- Console components ---> */ 

    void getScreenSize(int& width, int& height);
    void removeScrollbar();
    void setCursorPosition(int x, int y);
    void setBackgroundColor(int color);
    void applyConsoleSize(int width, int height);

    enum class AnchorPosition : int {
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight
    };
    void anchorConsole(AnchorPosition position);

private:
    std::vector<int> consoleSize; // [width, height] 
};
