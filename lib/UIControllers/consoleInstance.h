#pragma once
#include <iostream>
#include <vector>

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
    #include <wincon.h>
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

    void initializeConsole();

    std::vector<int> getConsoleSize(int& width, int& height);
    void setConsoleSize(int width, int height);

    int getCursorPosition(int& x, int& y);
    
    void getScreenSize(int& width, int& height);
    void removeScrollbar();
    void setCursorPosition(int x, int y);
    void setBackgroundColor(int color);
    void setTextColor(int color);

    void applyConsoleSize(int width, int height);
    void anchorConsoleTopLeft();

private:
    std::vector<int> consoleSize; // [width, height] 
};
