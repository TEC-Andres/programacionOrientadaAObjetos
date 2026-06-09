#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <string>
#include <ostream>
#include <functional>
#include "UIControllers/_component.h"

namespace ui {

/**
 * @brief A text box component for terminal UIs.
 * TextBox is a focusable component that allows user input. It supports a maximum length, optional password mode (which masks input), and customizable colors for background, foreground, and border. The component can be aligned within its container and has callbacks for when the text changes or is submitted.
 * Constructor parameters:
 * @param maxLength The maximum number of characters allowed in the text box.
 * @param bgColor The background color of the text box (hex string, e.g. "#1a73e8").
 * @param fgColor The foreground (text) color of the text box (hex string, e.g. "#ffffff").
 * @param borderColor The color of the text box border (hex string, e.g. "#888888").
 * @param passwordMode If true, the text box will mask input with '*' characters.
 * @param align The alignment of the text box within its container (default is Align::TopLeft).
 * Code example:
 * ```cpp
 * ui::TextBox txtBox(
 *   ui::maxLength=30,
 *   ui::bgColor="#2d2d2d",
 *   ui::fgColor="#00ff00",
 *   ui::borderColor="#555555",
 *   ui::passwordMode=false,
 *   ui::align=ui::Align::MiddleCenter
 * );
 * txtBox.setOnSubmit([](const std::string &text) {
 *   std::cout << "Submitted text: " << text << std::endl;
 * });
 * std::cout << txtBox.toString();
 * ```
 */
class TextBox : public ComponentBase {
public:
    TextBox(
        int maxLength = 50,
        const std::string &bgColor = "#1a73e8",
        const std::string &fgColor = "#ffffff",
        const std::string &borderColor = "#888888",
        bool passwordMode = false,
        Align align = Align::TopLeft
    );

    template<typename... NPs,
             typename = std::enable_if_t<(is_param_v<std::decay_t<NPs>> || ...)>>
    TextBox(NPs&&... params)
        : ComponentBase(30, 3)
        , maxLength_(50)
        , bgColor_("#1a73e8")
        , fgColor_("#ffffff")
        , borderColor_("#888888")
        , passwordMode_(false)
        , selected_(false)
    {
        align_ = Align::TopLeft;
        (apply_param(std::forward<NPs>(params)), ...);
    }

    std::string toString() const override;

    bool isFocusable() const override { return true; }
    void setSelected(bool s) override { selected_ = s; }
    bool selected() const override { return selected_; }
    bool handleKey(int key) override;

    void setText(const std::string &text) { text_ = text; }
    const std::string &text() const { return text_; }

    void setOnSubmit(std::function<void(const std::string&)> cb) { onSubmit_ = cb; }
    void setOnChange(std::function<void(const std::string&)> cb) { onChange_ = cb; }

private:
    using ComponentBase::apply_param;
    void apply_param(Param<int, width_tag> p) { width_ = p.value; }
    void apply_param(Param<int, height_tag> p) { height_ = p.value; }
    void apply_param(Param<std::string, bgColor_tag> p) { bgColor_ = p.value; }
    void apply_param(Param<std::string, fgColor_tag> p) { fgColor_ = p.value; }
    void apply_param(Param<std::string, borderColor_tag> p) { borderColor_ = p.value; }
    void apply_param(Param<bool, selected_tag> p) { selected_ = p.value; }
    void apply_param(Param<Align, align_tag> p) { align_ = p.value; }
    void apply_param(Param<float, displacementX_tag> p) { displacementX_ = p.value; }
    void apply_param(Param<float, displacementY_tag> p) { displacementY_ = p.value; }
    void apply_param(Param<int, maxLength_tag> p) { maxLength_ = p.value; }
    void apply_param(Param<bool, passwordMode_tag> p) { passwordMode_ = p.value; }

    std::string text_;
    int maxLength_;
    std::string bgColor_;
    std::string fgColor_;
    std::string borderColor_;
    bool passwordMode_;
    bool selected_;
    std::function<void(const std::string&)> onSubmit_;
    std::function<void(const std::string&)> onChange_;
};

} // namespace ui

#endif // TEXTBOX_H
