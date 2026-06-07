#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <string>
#include <ostream>
#include "UIControllers/_component.h"

namespace ui {

/**
 * @brief A background color component for terminal UIs.
 * 
 * Background is a non-focusable component that fills the entire terminal with a solid hex color using OSC 11 escape sequences.
 * Constructor parameters:
 * @param hexColor The hex color string for the background (e.g. "#1a1a2e").
 * Code example:
 * ```cpp
 * ui::Background bg("#1a1a2e");
 * ```
 */
class Background : public ComponentBase {
public:
    Background(const std::string &hexColor = "#000000");

    template<typename... NPs,
             typename = std::enable_if_t<(is_param_v<std::decay_t<NPs>> || ...)>>
    Background(NPs&&... params)
        : ComponentBase()
    {
        color_ = "#000000";
        (apply_param(std::forward<NPs>(params)), ...);
    }

    std::string toString() const override;

    void setColor(const std::string &hex) { color_ = hex; }
    std::string color() const { return color_; }

private:
    using ComponentBase::apply_param;
    void apply_param(Param<std::string, bgColor_tag> p) { color_ = p.value; }

    std::string color_;
};

} // namespace ui

#endif // BACKGROUND_H
