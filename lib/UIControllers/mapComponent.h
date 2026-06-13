#pragma once

#include <functional>
#include <vector>
#include <string>
#include <ostream>
#include "_component.h"

#define UP_ARROW    0x100
#define DOWN_ARROW  0x101
#define LEFT_ARROW  0x102
#define RIGHT_ARROW 0x103

namespace ui {

class Partition;

class MapComponent {
public:
    using RenderCallback = std::function<void(std::ostream &)>;

    MapComponent(int gridColumns = 3);

    void add(const RenderCallback &callback);

    template <typename T>
    void bind(T &object)
    {
        add([&object](std::ostream &out) { object.render(out); });
    }

    void attach(IComponent *component);
    void place(int row, int col, IComponent *component);

    bool moveUp();
    bool moveDown();
    bool moveLeft();
    bool moveRight();
    bool activate();

    void setPartition(Partition *p) { partition_ = p; }

    void render(std::ostream &out);
    bool handleInput();

    void run();
    void stop();

    void update(); // wake the event loop to trigger a redraw

    void setBackground(const std::string &hexColor) { background_ = hexColor; }
    const std::string &background() const { return background_; }

private:
    struct GridCell {
        IComponent *component;
        int row;
        int col;
    };

    void focusFirstFocusable();
    GridCell *findFocusable(int fromRow, int fromCol, int dRow, int dCol);
    void drainInputBuffer_();
    void clearScreen_();
    void updateConsoleSize_();

    std::vector<RenderCallback> callbacks_;
    std::vector<GridCell> cells_;
    bool running_;
    bool needsRedraw_;
    int focusRow_;
    int focusCol_;
    bool hasFocus_;
    int consoleWidth_;
    int consoleHeight_;
    int gridColumns_;
    int attachedCount_;
    std::string background_;
    Partition *partition_ = nullptr;
#if defined(_WIN32) || defined(_WIN64)
    void *wakeEvent_ = nullptr;
#elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    int wakePipe_[2];
#endif
};

} // namespace ui
