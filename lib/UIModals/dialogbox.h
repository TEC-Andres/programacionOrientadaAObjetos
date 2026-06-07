#ifndef DIALOGBOX_H
#define DIALOGBOX_H

#include <string>
#include <vector>
#include <ostream>
#include "button.h"
#include "UIControllers/_component.h"

#if defined(_WIN32) && defined(DialogBox)
    #undef DialogBox
#endif

namespace ui {

struct DialogButton {
    std::string text;
    bool selected;
    DialogButton(const std::string &t = "", bool s = false) : text(t), selected(s) {}
};

/**
 * @brief A dialog box component for terminal UIs.
 * 
 * DialogBox is a focusable component that displays a title and a message body along with interactive buttons. It can be used to display messages and prompt the user for input.
 * 
 * @param title The title of the dialog box.
 * @param message The message body of the dialog box.
 * @param width The width of the dialog box in characters.
 * @param height The height of the dialog box in characters.
 * @param titleBgColor The background color of the title area (hex string, e.g "#1a73e8").
 * @param titleFgColor The foreground (text) color of the title area (hex string, e.g. "#ffffff").
 * @param bodyBgColor The background color of the message body (hex string, e.g. "#ffffff").
 * @param bodyFgColor The foreground (text) color of the message body (hex string, e.g. "#333333").
 * @param borderColor The color of the dialog box border (hex string, e.g. "#888888").
 * @param align The alignment of the dialog box within its container (default is Align::TopLeft).
 */
class DialogBox : public ComponentBase {
public:
    DialogBox(
        const std::string &title = "",
        const std::string &message = "",
        int width = 60,
        int height = 12,
        const std::string &titleBgColor = "#1a73e8",
        const std::string &titleFgColor = "#ffffff",
        const std::string &bodyBgColor = "#ffffff",
        const std::string &bodyFgColor = "#333333",
        const std::string &borderColor = "#888888",
        Align align = Align::TopLeft
    );

    template<typename... NPs,
             typename = std::enable_if_t<(is_param_v<std::decay_t<NPs>> || ...)>>
    DialogBox(NPs&&... params)
        : ComponentBase(60, 12)
        , title_("")
        , message_("")
        , titleBgColor_("#1a73e8")
        , titleFgColor_("#ffffff")
        , bodyBgColor_("#ffffff")
        , bodyFgColor_("#333333")
        , borderColor_("#888888")
    {
        align_ = Align::TopLeft;
        (apply_param(std::forward<NPs>(params)), ...);
    }

    void addButton(const std::string &text, bool selected = false);
    void clearButtons();

    std::string toString() const override;

private:
    using ComponentBase::apply_param;
    void apply_param(Param<std::string, title_tag> p) { title_ = p.value; }
    void apply_param(Param<std::string, message_tag> p) { message_ = p.value; }
    void apply_param(Param<std::string, titleBgColor_tag> p) { titleBgColor_ = p.value; }
    void apply_param(Param<std::string, titleFgColor_tag> p) { titleFgColor_ = p.value; }
    void apply_param(Param<std::string, bodyBgColor_tag> p) { bodyBgColor_ = p.value; }
    void apply_param(Param<std::string, bodyFgColor_tag> p) { bodyFgColor_ = p.value; }
    void apply_param(Param<std::string, borderColor_tag> p) { borderColor_ = p.value; }

    std::string title_;
    std::string message_;
    std::string titleBgColor_;
    std::string titleFgColor_;
    std::string bodyBgColor_;
    std::string bodyFgColor_;
    std::string borderColor_;
    std::vector<DialogButton> buttons_;
};

} // namespace ui

#endif // DIALOGBOX_H
