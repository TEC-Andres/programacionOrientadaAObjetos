#ifndef TERMINALAREA_H
#define TERMINALAREA_H

#include <string>
#include <vector>
#include <ostream>
#include <deque>
#include <functional>
#include "UIControllers/_component.h"

namespace ui {

class TerminalArea : public ComponentBase {
public:
    TerminalArea(
        int width = 50,
        int height = 8,
        const std::string &bgColor = "#0d1117",
        const std::string &fgColor = "#00ff00",
        const std::string &borderColor = "#30363d",
        const std::string &promptColor = "#00ff00",
        const std::string &outputColor = "#8b949e",
        const std::string &errorColor = "#ff6b6b"
    );

    template<typename... NPs,
             typename = std::enable_if_t<(is_param_v<std::decay_t<NPs>> || ...)>>
    TerminalArea(NPs&&... params)
        : ComponentBase(50, 8)
        , bgColor_("#0d1117")
        , fgColor_("#00ff00")
        , borderColor_("#30363d")
        , promptColor_("#00ff00")
        , outputColor_("#8b949e")
        , errorColor_("#ff6b6b")
    {
        align_ = Align::TopLeft;
        (apply_param(std::forward<NPs>(params)), ...);
    }

    void addOutput(const std::string &text, bool isError = false);
    void addCommand(const std::string &cmd);
    void setOnCommand(std::function<void(const std::string&)> cb) { onCommand_ = cb; }
    void clear();

    std::string toString() const override;

    bool isFocusable() const override { return true; }
    void setSelected(bool s) override { if (s != selected_) { selected_ = s; setDirty(); } }
    bool selected() const override { return selected_; }
    bool handleKey(int key) override;
    void onActivate() override { if (onActivate_) onActivate_(); }
    void setOnActivate(std::function<void()> cb) override { onActivate_ = cb; }
    bool wantsCursor() const override { return selected_; }
    int cursorX() const override;
    int cursorY() const override;

private:
    using ComponentBase::apply_param;
    void apply_param(Param<std::string, bgColor_tag> p) { bgColor_ = p.value; }
    void apply_param(Param<std::string, fgColor_tag> p) { fgColor_ = p.value; }
    void apply_param(Param<std::string, borderColor_tag> p) { borderColor_ = p.value; }
    void apply_param(Param<std::string, titleFgColor_tag> p) { promptColor_ = p.value; }
    void apply_param(Param<std::string, bodyFgColor_tag> p) { outputColor_ = p.value; }
    void apply_param(Param<std::string, borderFgColor_tag> p) { errorColor_ = p.value; }
    void apply_param(Param<int, width_tag> p) { width_ = p.value; }
    void apply_param(Param<int, height_tag> p) { height_ = p.value; }

    std::string bgColor_;
    std::string fgColor_;
    std::string borderColor_;
    std::string promptColor_;
    std::string outputColor_;
    std::string errorColor_;

    struct Line {
        std::string text;
        bool isError;
        bool isPrompt;
        Line(const std::string &t, bool e = false, bool p = false)
            : text(t), isError(e), isPrompt(p) {}
    };
    std::deque<Line> lines_;
    int maxLines_ = 100;
    std::function<void(const std::string&)> onCommand_;
    std::string currentInput_;
    bool selected_ = false;
    std::function<void()> onActivate_;
};

} // namespace ui

#endif // TERMINALAREA_H
