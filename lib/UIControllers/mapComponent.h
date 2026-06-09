#pragma once

#include <functional>
#include <vector>
#include <string>
#include <ostream>
#include <chrono>
#include <thread>
#include "_component.h"

#define UP_ARROW    0x100
#define DOWN_ARROW  0x101
#define LEFT_ARROW  0x102
#define RIGHT_ARROW 0x103

namespace ui {

class Partition;

/**
 * @brief Construct a new MapComponent object.
 * Initializes the MapComponent with an optional frames-per-second (FPS) setting for rendering. The constructor sets up internal state for managing render callbacks, grid cells, focus, and input handling.
 * @param framesPerSecond The desired frames per second for rendering the map (default: 30). Must be a positive integer; otherwise, it defaults to 30 FPS.
 * ## Example
 * ```cpp
 * ui::MapComponent map(60); // Create a MapComponent with 60 FPS
 * ui::MapComponent defaultMap; // Create a MapComponent with default 30 FPS
 * ```
*/
class MapComponent {
public:
    using RenderCallback = std::function<void(std::ostream &)>;

    MapComponent(int framesPerSecond = 30, int gridColumns = 3);

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
    void removeScrollbar_();

    std::vector<RenderCallback> callbacks_;
    std::vector<GridCell> cells_;
    int fps_;
    bool running_;
    std::string lastFrame_;
    int focusRow_;
    int focusCol_;
    bool hasFocus_;
    bool firstFrame_;
    int lastConsoleWidth_;
    int lastConsoleHeight_;
    int gridColumns_;
    int attachedCount_;
    std::string background_;
    Partition *partition_ = nullptr;
};

} // namespace ui
