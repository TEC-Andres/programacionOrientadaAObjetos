#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <string>
#include <vector>
#include <ostream>
#include "UIControllers/_component.h"

#if defined(_WIN32) && defined(MessageBox)
    #undef MessageBox
#endif

namespace ui {

/**
 * @brief A message box component for terminal UIs.
 * 
 * MessageBox is a non-focusable component that displays a title and a message body. It can be used to show information or alerts to the user.  
 * Constructor parameters:  
 * @param title The title of the message box.
 * @param message The message body of the message box.
 * @param width The width of the message box in characters.
 * @param height The height of the message box in characters.
 * @param titleBgColor The background color of the title area (hex string, e.g "#1a73e8").
 * @param titleFgColor The foreground (text) color of the title area (hex string, e.g. "#ffffff").
 * @param bodyBgColor The background color of the message body (hex string, e.g. "#ffffff").
 * @param bodyFgColor The foreground (text) color of the message body (hex string, e.g. "#333333").
 * @param borderColor The color of the message box border (hex string, e.g. "#888888").
 * @param align The alignment of the message box within its container (default is Align::TopLeft).
 * Code example:
 * ```cpp
 * ui::MessageBox msgBox(
 *   ui::title="Error",
 *   ui::message="An unexpected error occurred. Please try again later.",
 *   ui::width=50,
 *   ui::height=8,
 *   ui::bgColor="#ff0000",
 *   ui::fgColor="#ffffff",
 *   ui::borderColor="#ff6666",
 *   ui::align=ui::Align::MiddleCenter
 * );
 * std::cout << msgBox.toString();
 * ```
 */
class MessageBox : public ComponentBase {
public:
    MessageBox(
        const std::string &title = "",
        const std::string &message = "",
        int width = 60,
        int height = 10,
        const std::string &titleBgColor = "#1a73e8",
        const std::string &titleFgColor = "#ffffff",
        const std::string &bodyBgColor = "#ffffff",
        const std::string &bodyFgColor = "#333333",
        const std::string &borderColor = "#888888",
        Align align = Align::TopLeft
    );

    template<typename... NPs,
             typename = std::enable_if_t<(is_param_v<std::decay_t<NPs>> || ...)>>
    MessageBox(NPs&&... params)
        : ComponentBase(60, 10)
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

    std::string toString() const override;

    void setTitle(const std::string &title) { title_ = title; }
    void setMessage(const std::string &message) { message_ = message; }

private:
    using ComponentBase::apply_param;
    void apply_param(Param<std::string, title_tag> p) { title_ = p.value; }
    void apply_param(Param<std::string, message_tag> p) { message_ = p.value; }
    void apply_param(Param<std::string, bgColor_tag> p) { titleBgColor_ = p.value; }
    void apply_param(Param<std::string, fgColor_tag> p) { titleFgColor_ = p.value; }
    void apply_param(Param<std::string, borderColor_tag> p) { bodyBgColor_ = p.value; }
    void apply_param(Param<std::string, borderFgColor_tag> p) { bodyFgColor_ = p.value; }
    void apply_param(Param<std::string, borderBgColor_tag> p) { borderColor_ = p.value; }
    void apply_param(Param<std::string, titleBgColor_tag> p) { titleBgColor_ = p.value; }
    void apply_param(Param<std::string, titleFgColor_tag> p) { titleFgColor_ = p.value; }
    void apply_param(Param<std::string, bodyBgColor_tag> p) { bodyBgColor_ = p.value; }
    void apply_param(Param<std::string, bodyFgColor_tag> p) { bodyFgColor_ = p.value; }

    std::string title_;
    std::string message_;
    std::string titleBgColor_;
    std::string titleFgColor_;
    std::string bodyBgColor_;
    std::string bodyFgColor_;
    std::string borderColor_;
};

} // namespace ui

#endif // MESSAGEBOX_H
