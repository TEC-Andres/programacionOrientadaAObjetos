#include "dynamicLink.h"

namespace ui {

namespace {

void getFrameSize(const std::string &text, int &width, int &height)
{
    width = 0;
    height = 0;

    int current = 0;
    bool endedWithNewline = false;
    for (const char ch : text) {
        if (ch == '\n') {
            width = std::max(width, current);
            current = 0;
            ++height;
            endedWithNewline = true;
            continue;
        }
        if (ch == '\r') {
            continue;
        }
        ++current;
        endedWithNewline = false;
    }

    if ((!text.empty() || current > 0) && !endedWithNewline) {
        width = std::max(width, current);
        ++height;
    }
}

} // namespace

DynamicLink::DynamicLink(int framesPerSecond)
    : callbacks_(),
      fps_(framesPerSecond > 0 ? framesPerSecond : 30),
      running_(false),
      last_frame_(),
      last_console_width_(0),
      last_console_height_(0),
      console_()
{
}

void DynamicLink::add(const RenderCallback &callback)
{
    callbacks_.push_back(callback);
}

void DynamicLink::clearScreen() const
{
    std::cout << "\x1b[2J\x1b[H";
}

void DynamicLink::mainloop()
{
    mainloop(0);
}

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

        int width = 0;
        int height = 0;
        getFrameSize(frameText, width, height);
        width = std::max(1, width);
        height = std::max(1, height);
        if (width != last_console_width_ || height != last_console_height_) {
            console_.applyConsoleSize(width, height);
            last_console_width_ = width;
            last_console_height_ = height;
            last_frame_.clear();
        }

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

void DynamicLink::stop()
{
    running_ = false;
}

} // namespace ui
