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

namespace ui {

/**
 * @brief A dynamic rendering loop for terminal applications.
 * DynamicLink provides a simple framework for creating dynamic terminal applications that require continuous rendering and user input handling. It allows you to register multiple render callbacks that will be called each frame to generate the output. The main loop handles timing to achieve a specified frames-per-second (FPS) rate and manages console output efficiently by only redrawing when the rendered frame changes.
 * 
 * Constructor parameters:
 * @param framesPerSecond The desired frames per second for rendering the output (default: 30). Must be a positive integer; otherwise, it defaults to 30 FPS.
 * Example usage:
 * ```cpp
 * ui::DynamicLink link(60); // Create a DynamicLink with 60 FPS
 * link.add([](std::ostream &out) {
 *  out << "Rendering frame content" << std::endl;
 * });
 * link.mainloop(); // Start the main loop
 * ```
 */
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
};

} // namespace ui

#endif // DYNAMIC_LINK_H
