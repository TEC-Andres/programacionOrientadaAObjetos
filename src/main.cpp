#include "UIControllers/consoleInstance.h"
#include <iostream>

int main() {
    ConsoleInstance console;

    int width, height;
    console.getConsoleSize(width, height);
    std::cout << "Console Size: " << width << "x" << height << std::endl;

    return 0;
}