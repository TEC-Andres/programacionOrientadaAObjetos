#include "dynamicLink.h"

namespace ui {

DynamicLink::DynamicLink(int framesPerSecond)
    : callbacks_(),
      fps_(framesPerSecond > 0 ? framesPerSecond : 30),
      running_(false),
      last_frame_()
{
}

/**
 * @brief Add a render callback to the DynamicLink.
 * Registers a new render callback function that will be called during the rendering process. The callback should accept a reference to an output stream where it can write its rendered content.
 * @param callback A function or lambda that takes an `std::ostream&` parameter and renders content to it.
 * ## Example
 * ```cpp * ui::DynamicLink link;
 * link.add([](std::ostream &out) {
 *   out << "Rendering frame content" << std::endl;
 * });
 * ```
 */
void DynamicLink::add(const RenderCallback &callback)
{
    callbacks_.push_back(callback);
}

/**
 * @brief Clear the console screen.
 * This method sends the appropriate ANSI escape codes to clear the console screen and reset the cursor position to the top-left corner. It is used internally by the DynamicLink to ensure that each new frame is rendered on a clean slate.
 */
void DynamicLink::clearScreen() const
{
    std::cout << "\x1b[2J\x1b[H";
}

/**
 * @brief Main loop basic function.
 */
void DynamicLink::mainloop()
{
    mainloop(0);
}

/**
 * @brief Main loop for the DynamicLink.
 * This method starts the main loop of the DynamicLink, which continuously calls the registered render callbacks to generate the output frame. The loop runs until `stop()` is called or until the specified frame limit is reached (if `frameLimit` is greater than 0). The method also handles frame timing based on the configured frames per second (FPS) and ensures that the console output is updated only when there are changes to the rendered content.
 * @param frameLimit The maximum number of frames to render before automatically stopping the loop. If set to 0 or a negative value, the loop will run indefinitely until `stop()` is called.
 * ## Example
 * ```cpp
 * ui::DynamicLink link(60); // Create a DynamicLink with 60 FPS
 * link.add([](std::ostream &out) {
 *   out << "Rendering frame content" << std::endl;
 * });
 * link.mainloop(100); // Run the main loop for 100 frames
 * ```
 */
void DynamicLink::mainloop(int frameLimit)
{
    running_ = true;
    const auto frameDelay = std::chrono::milliseconds(1000 / fps_);
    int frame = 0;

    while (running_ && (frameLimit <= 0 || frame < frameLimit)) {
        std::ostringstream buffer;
        for (const auto &callback : callbacks_) {
            callback(buffer);
        }
        const std::string frameText = buffer.str();

        if (frameText != last_frame_) {
            clearScreen();
            std::cout << frameText;
            std::cout.flush();
            last_frame_ = frameText;
        }
        ++frame;
        if (running_ && frameDelay.count() > 0) {
            std::this_thread::sleep_for(frameDelay);
        }
    }
}

/**
 * @brief Stop the DynamicLink main loop.
 * This method stops the main loop of the DynamicLink, causing it to exit after the current frame is rendered.
 */
void DynamicLink::stop()
{
    running_ = false;
}

} // namespace ui
