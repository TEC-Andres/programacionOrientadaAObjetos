#pragma once

#include <ostream>
#include <functional>
#include <string>
#include <vector>
#include "params.h"

#define UI_KEY_UP    0x100
#define UI_KEY_DOWN  0x101
#define UI_KEY_LEFT  0x102
#define UI_KEY_RIGHT 0x103

namespace ui {

using namespace component;

enum class Align {
    Left, Center, Right,
    TopLeft, TopCenter, TopRight,
    MiddleLeft, MiddleCenter, MiddleRight,
    BottomLeft, BottomCenter, BottomRight
};

namespace component {
using ui::Align;
}

namespace component {

struct align_tag {
    Param<Align, align_tag> operator=(Align v) const { return {v}; }
};
inline constexpr align_tag align{};

struct segment1Align_tag {
    Param<Align, segment1Align_tag> operator=(Align v) const { return {v}; }
};
inline constexpr segment1Align_tag segment1Align{};

struct segment2Align_tag {
    Param<Align, segment2Align_tag> operator=(Align v) const { return {v}; }
};
inline constexpr segment2Align_tag segment2Align{};

struct segment3Align_tag {
    Param<Align, segment3Align_tag> operator=(Align v) const { return {v}; }
};
inline constexpr segment3Align_tag segment3Align{};

} // namespace component

/**
 * @brief A helper class for rendering UI components.
 * RenderHelper provides utility functions for parsing colors, generating ANSI escape codes for foreground and background colors, resetting text attributes, and wrapping text to a specified width. It is used internally by UI components to assist with rendering their visual representation in the console.
 */
class RenderHelper {
public:
    bool parseHex(const std::string &hex, int &r, int &g, int &b) const;
    std::string fg(int r, int g, int b) const;
    std::string bg(int r, int g, int b) const;
    std::string reset() const;
    std::vector<std::string> wrapText(const std::string &text, int maxWidth) const;
};

/**
 * @brief Interface for UI components.
 * IComponent defines the common interface for all UI components in the library. It includes methods for rendering, getting dimensions, handling focus and activation, and managing alignment and displacement. All UI components should inherit from this interface to ensure consistent behavior and interoperability within the UI framework.
 */
class IComponent {
public:
    virtual ~IComponent() = default;
    virtual void render(std::ostream &out) = 0;
    virtual std::string toString() const = 0;
    virtual int width() const = 0;
    virtual int height() const = 0;
    virtual bool isFocusable() const { return false; }
    virtual void setSelected(bool) {}
    virtual bool selected() const { return false; }
    virtual void onActivate() {}
    virtual void setOnActivate(std::function<void()>) {}
    virtual bool handleKey(int /*key*/) { return false; }
    virtual bool usesExternalPositioning() const { return false; }
    virtual void setUsesExternalPositioning(bool) {}
    Align alignment() const { return align_; }
    void setAlignment(Align a) { align_ = a; }
    float displacementX() const { return displacementX_; }
    void setDisplacementX(float pct) { displacementX_ = pct; }
    float displacementY() const { return displacementY_; }
    void setDisplacementY(float pct) { displacementY_ = pct; }

protected:
    Align align_ = Align::Left;
    float displacementX_ = 0.0f;
    float displacementY_ = 0.0f;
};

/**
 * @brief Base class for UI components.
 * ComponentBase provides a common implementation for UI components, including properties for width, height, alignment, and displacement. It also includes a RenderHelper instance for assisting with rendering tasks. ComponentBase implements the IComponent interface and provides default implementations for rendering and dimension management. Specific UI components can inherit from ComponentBase to leverage its functionality and override methods as needed to create custom behavior and appearance.
 */
class ComponentBase : public IComponent {
public:
    ComponentBase(int defaultWidth = 0, int defaultHeight = 0)
        : width_(defaultWidth), height_(defaultHeight) {}

    int width() const override { return width_; }
    int height() const override { return height_; }
    void setWidth(int w) { width_ = w; }
    void setHeight(int h) { height_ = h; }

    void render(std::ostream &out) override;

    static int getConsoleWidth();
    static int getConsoleHeight();
    static void enableVT();

protected:
    void apply_param(Param<int, width_tag> p) { width_ = p.value; }
    void apply_param(Param<int, height_tag> p) { height_ = p.value; }
    void apply_param(Param<Align, align_tag> p) { align_ = p.value; }
    void apply_param(Param<float, displacementX_tag> p) { displacementX_ = p.value; }
    void apply_param(Param<float, displacementY_tag> p) { displacementY_ = p.value; }

    RenderHelper render_;
    int width_;
    int height_;
};

} // namespace ui
