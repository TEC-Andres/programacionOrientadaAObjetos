#ifndef BUTTON_H
#define BUTTON_H

#include <string>
#include <ostream>
#include <functional>
#include "UIControllers/_component.h"

namespace ui {

/**
 * @brief A button component for terminal UIs.
 * 
 * Button is a focusable component that displays a clickable button. It can be selected and activated by the user.
 * Constructor parameters:
 * @param text The text displayed on the button.
 * @param width The width of the button in characters.
 * @param bgColor The background color of the button (hex string, e.g. "#1a73e8").
 * @param fgColor The foreground (text) color of the button (hex string, e.g. "#ffffff").
 * @param borderColor The color of the button border (hex string, e.g. "#888888").
 * @param selected Whether the button is initially selected (focused).
 * @param align The alignment of the button within its container (default is Align::TopLeft).
 * Code example:
 * ```cpp
 * ui::Button btn(" Click Me ", 20, "#1a73e8", "#ffffff", "#888888", false, ui::Align::MiddleCenter);
 * btn.setOnActivate([]() { std::cout << "Button clicked!\n"; });
 * ```
 */
class Button : public ComponentBase {
public:
    Button(
        const std::string &text = "Button",
        int width = 20,
        const std::string &bgColor = "#1a73e8",
        const std::string &fgColor = "#ffffff",
        const std::string &borderColor = "#888888",
        bool selected = false,
        Align align = Align::TopLeft
    );

    template<typename... NPs,
             typename = std::enable_if_t<(is_param_v<std::decay_t<NPs>> || ...)>>
    Button(NPs&&... params)
        : ComponentBase(20, 3)
        , text_("Button")
        , bgColor_("#1a73e8")
        , fgColor_("#ffffff")
        , borderColor_("#888888")
        , selected_(false)
    {
        align_ = Align::TopLeft;
        (apply_param(std::forward<NPs>(params)), ...);
    }

    std::string toString() const override;

    void setText(const std::string &text) { text_ = text; }
    void setSelected(bool s) override { if (s != selected_) { selected_ = s; setDirty(); } }
    bool selected() const override { return selected_; }
    bool isFocusable() const override { return true; }
    void onActivate() override { if (onActivate_) onActivate_(); }
    void setOnActivate(std::function<void()> cb) override { onActivate_ = cb; }

private:
    using ComponentBase::apply_param;
    void apply_param(Param<std::string, text_tag> p) { text_ = p.value; }
    void apply_param(Param<std::string, bgColor_tag> p) { bgColor_ = p.value; }
    void apply_param(Param<std::string, fgColor_tag> p) { fgColor_ = p.value; }
    void apply_param(Param<std::string, borderColor_tag> p) { borderColor_ = p.value; }
    void apply_param(Param<bool, selected_tag> p) { selected_ = p.value; }

    std::string text_;
    std::string bgColor_;
    std::string fgColor_;
    std::string borderColor_;
    bool selected_;
    std::function<void()> onActivate_;
};

} // namespace ui

#endif // BUTTON_H
