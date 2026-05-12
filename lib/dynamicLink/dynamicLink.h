#ifndef DYNAMIC_LINK_H
#define DYNAMIC_LINK_H

#include <functional>
#include <ostream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <sstream>
#include <thread>


#include "UIControllers/consoleInstance.h"

namespace ui {

class DynamicLink {
public:
    using RenderCallback = std::function<void(std::ostream &)>;

    DynamicLink(int framesPerSecond = 30);

    void add(const RenderCallback &callback);

    template <typename T>
    void bind(T &object)
    {
        add([&object](std::ostream &out) { object.render(out); });
    }

    void mainloop();
    void mainloop(int frameLimit);
    void stop();

private:
    void clearScreen() const;

    std::vector<RenderCallback> callbacks_;
    int fps_;
    bool running_;
    std::string last_frame_;
    int last_console_width_;
    int last_console_height_;
    ConsoleInstance console_;
};

} // namespace ui

#endif // DYNAMIC_LINK_H
