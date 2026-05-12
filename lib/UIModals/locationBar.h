#ifndef LOCATION_BAR_H
#define LOCATION_BAR_H

#include <string>
#include <vector>
#include <ostream>
#include "UIControllers/consoleInstance.h"

#define SHOW 1
#define HIDE 0

namespace ui {

enum class Align { Left, Center, Right };

struct Segment {
    int visible;
    std::string text;
    Align align;
    Segment(int v = SHOW, const std::string &t = "", Align a = Align::Left) : visible(v), text(t), align(a) {}
};

class LocationBar : public ::ConsoleInstance {
public:
    LocationBar(const std::string &bgColor = "#ffffff",
                const std::string &fgColor = "#000000",
                int segment1 = SHOW, const std::string &text1 = "", Align location1 = Align::Left,
                int segment2 = SHOW, const std::string &text2 = "", Align location2 = Align::Center,
                int segment3 = HIDE, const std::string &text3 = "", Align location3 = Align::Right,
                int width = 0);

    // Render the bar to an output stream (console, file, etc.)
    void render(std::ostream &out);

    // Return the bar as a string
    std::string toString() const;

    // Accessors
    void setWidth(int w) { width_ = w; }
    int width() const { return width_; }

private:
    std::string bgColor_;
    std::string fgColor_;
    std::vector<Segment> segments_;
    int width_;
};

} // namespace ui

#endif // LOCATION_BAR_H
