#ifndef MAP_COMPONENT_H
#define MAP_COMPONENT_H

#include <functional>
#include <vector>
#include <string>
#include <ostream>
#include <chrono>
#include <thread>
#include "_component.h"

#define UP_ARROW    72
#define DOWN_ARROW  80
#define LEFT_ARROW  75
#define RIGHT_ARROW 77

namespace ui {

class MapComponent {
public:
    using RenderCallback = std::function<void(std::ostream &)>;

    MapComponent(int framesPerSecond = 30);

    void add(const RenderCallback &callback);

    template <typename T>
    void bind(T &object)
    {
        add([&object](std::ostream &out) { object.render(out); });
    }

    void place(int row, int col, IComponent *component);

    bool moveUp();
    bool moveDown();
    bool moveLeft();
    bool moveRight();
    bool activate();

    void render(std::ostream &out);
    bool handleInput();

    void run();
    void stop();

private:
    struct GridCell {
        IComponent *component;
        int row;
        int col;
    };

    void clearScreen() const;
    void focusFirstFocusable();
    GridCell *findFocusable(int fromRow, int fromCol, int dRow, int dCol);

    std::vector<RenderCallback> callbacks_;
    std::vector<GridCell> cells_;
    int fps_;
    bool running_;
    std::string lastFrame_;
    int focusRow_;
    int focusCol_;
    bool hasFocus_;
};

} // namespace ui

#endif // MAP_COMPONENT_H
