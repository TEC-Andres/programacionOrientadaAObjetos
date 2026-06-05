#pragma once

#include <ostream>
#include <functional>
#include <string>
#include <vector>

namespace ui {

class RenderHelper {
public:
    bool parseHex(const std::string &hex, int &r, int &g, int &b) const;
    std::string fg(int r, int g, int b) const;
    std::string bg(int r, int g, int b) const;
    std::string reset() const;
    std::vector<std::string> wrapText(const std::string &text, int maxWidth) const;
};

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
};

} // namespace ui
