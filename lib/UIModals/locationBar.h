#ifndef LOCATION_BAR_H
#define LOCATION_BAR_H

#include <string>
#include <vector>
#include <ostream>
#include "UIControllers/_component.h"

#define SHOW 1
#define HIDE 0

namespace ui {

struct Segment {
    int visible;
    std::string text;
    Align align;
    Segment(int v = SHOW, const std::string &t = "", Align a = Align::Left) : visible(v), text(t), align(a) {}
};

/**
 * @brief A location bar component for terminal UIs.
 * LocationBar is a non-focusable component that displays up to three segments of text, each with its own alignment (left, center, right).  
 * Constructor parameters:
 * @param bgColor The background color of the location bar (hex string, e.g., "#ffffff").
 * @param fgColor The foreground (text) color of the location bar (hex string, e.g., "#000000").
 * @param segment1 Whether the first segment is visible (SHOW/HIDE), its text, and its alignment.
 * @param segment2 Whether the second segment is visible (SHOW/HIDE), its text, and its alignment.
 * @param segment3 Whether the third segment is visible (SHOW/HIDE), its text, and its alignment.
 * @param width The width of the location bar in characters (0 for auto).
 * Example usage:
 * ```cpp
 * ui::LocationBar bar(
 *   "#ffffff", "#000000",
 *   SHOW, "Left-aligned text", ui::Align::Left,
 *   SHOW, "Center-aligned text", ui::Align::Center,
 *   HIDE, "Right-aligned text", ui::Align::Right,
 *   0
 * );
 * ```
 */
class LocationBar : public ComponentBase {
public:
    LocationBar(const std::string &bgColor = "#ffffff",
                const std::string &fgColor = "#000000",
                int segment1 = SHOW, const std::string &text1 = "", Align location1 = Align::Left,
                int segment2 = SHOW, const std::string &text2 = "", Align location2 = Align::Center,
                int segment3 = HIDE, const std::string &text3 = "", Align location3 = Align::Right,
                int width = 0);

    template<typename... NPs,
             typename = std::enable_if_t<(is_param_v<std::decay_t<NPs>> || ...)>>
    LocationBar(NPs&&... params)
        : ComponentBase(0, 1)
        , bgColor_("#ffffff")
        , fgColor_("#000000")
    {
        segments_ = {
            Segment{SHOW, "", Align::Left},
            Segment{SHOW, "", Align::Center},
            Segment{HIDE, "", Align::Right}
        };
        (apply_param(std::forward<NPs>(params)), ...);
    }

    void render(std::ostream &out) override;
    std::string toString() const override;

private:
    using ComponentBase::apply_param;
    void apply_param(Param<std::string, bgColor_tag> p) { bgColor_ = p.value; }
    void apply_param(Param<std::string, fgColor_tag> p) { fgColor_ = p.value; }
    void apply_param(Param<int, segment1Visible_tag> p) { segments_[0].visible = p.value; }
    void apply_param(Param<std::string, segment1Text_tag> p) { segments_[0].text = p.value; }
    void apply_param(Param<Align, segment1Align_tag> p) { segments_[0].align = p.value; }
    void apply_param(Param<int, segment2Visible_tag> p) { segments_[1].visible = p.value; }
    void apply_param(Param<std::string, segment2Text_tag> p) { segments_[1].text = p.value; }
    void apply_param(Param<Align, segment2Align_tag> p) { segments_[1].align = p.value; }
    void apply_param(Param<int, segment3Visible_tag> p) { segments_[2].visible = p.value; }
    void apply_param(Param<std::string, segment3Text_tag> p) { segments_[2].text = p.value; }
    void apply_param(Param<Align, segment3Align_tag> p) { segments_[2].align = p.value; }

    std::string bgColor_;
    std::string fgColor_;
    std::vector<Segment> segments_;
};

} // namespace ui

#endif // LOCATION_BAR_H
