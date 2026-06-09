#pragma once

#include <string>
#include <vector>
#include <ostream>
#include <functional>
#include "UIControllers/_component.h"

namespace ui {

/**
 * @brief An object renderer component for terminal UIs.
 * 
 * ObjectRenderer is a non-focusable component that renders ASCII art, sprite-like objects, or text loaded from a file or provided as raw lines.
 * Constructor parameters:
 * @param filepath Path to a text file containing the object to render.
 * Code example:
 * ```cpp
 * ui::ObjectRenderer logo("assets/logo.txt");
 * ```
 * It can also be constructed with raw C-string arrays:
 * ```cpp
 * const char* const art[] = {
 *   "  /\\_/\\",
 *   " ( o.o )",
 *   "  > ^ <"
 * };
 * ui::ObjectRenderer obj(art, 3);
 * ```
 */
class ObjectRenderer : public ComponentBase {
public:
    ObjectRenderer(const std::string &filepath = "");
    ObjectRenderer(const char* const* rawLines, int lineCount);

    template<typename... NPs,
             typename = std::enable_if_t<(is_param_v<std::decay_t<NPs>> || ...)>>
    ObjectRenderer(const char* const* rawLines, int lineCount, NPs&&... params)
        : ComponentBase(0, 0)
        , maxWidth_(0)
    {
        lines_.reserve(lineCount);
        for (int i = 0; i < lineCount; ++i) {
            std::string line(rawLines[i]);
            lines_.push_back(line);
            int w = visibleWidth(line);
            if (w > maxWidth_) maxWidth_ = w;
        }
        width_ = maxWidth_;
        height_ = (int)lines_.size();
        (apply_param(std::forward<NPs>(params)), ...);
    }

    std::string toString() const override;

    void setFile(const std::string &filepath);
    void setCenter(bool center) { align_ = center ? Align::Center : Align::Left; }
    void setResizable(bool r) { resizable_ = r; }
    bool resizable() const { return resizable_; }
    int anchorX() const;
    int anchorY() const;

    bool usesExternalPositioning() const override { return externalPos_; }
    void setUsesExternalPositioning(bool e) override { externalPos_ = e; }

    void setOnActivate(std::function<void()> cb) override { onActivate_ = cb; }
    void onActivate() override { if (onActivate_) onActivate_(); }

private:
    using ComponentBase::apply_param;
    void apply_param(Param<bool, resizable_tag> p) { resizable_ = p.value; }

    std::string filepath_;
    std::vector<std::string> lines_;
    int maxWidth_;
    bool resizable_ = false;
    bool externalPos_ = false;
    std::function<void()> onActivate_;

    std::string stripAnsi(const std::string &line) const;
    int visibleWidth(const std::string &line) const;
    void loadFile();
};

} // namespace ui
