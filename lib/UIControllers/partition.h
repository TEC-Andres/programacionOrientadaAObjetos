#pragma once

#include <vector>
#include <ostream>
#include "_component.h"

namespace ui {

class Partition {
public:
    enum Type {
        OneSide,
        TwoSideH,
        TwoSideV,
        FourSide
    };

    class Region {
        friend class Partition;
    public:
        Region() : x_(0), y_(0), w_(0), h_(0), cachedW_(-1), cachedH_(-1) {}

        void attach(IComponent *comp) { comps_.push_back(comp); }
        void setBackground(const std::string &hex) { bgColor_ = hex; cachedW_ = -1; }
        const std::string& background() const { return bgColor_; }

        int x()  const { return x_; }
        int y()  const { return y_; }
        int width()  const { return w_; }
        int height() const { return h_; }

        const std::vector<IComponent*>& components() const { return comps_; }

    private:
        int x_, y_, w_, h_;
        std::vector<IComponent*> comps_;
        std::string bgColor_;
        std::string bgCache_;
        int cachedW_;
        int cachedH_;
    };

    Partition(Type type, float ratio1 = 100.0f, float ratio2 = 0.0f);

    Region& full();
    Region& left();
    Region& right();
    Region& top();
    Region& bottom();
    Region& topLeft();
    Region& topRight();
    Region& bottomLeft();
    Region& bottomRight();

    void update(int yOffset = 0);
    void render(std::ostream &out);

    bool handleKey(int key);
    void setFocus(bool on);
    bool hasFocus() const { return hasFocus_; }

private:
    static int alignmentBand(Align a);
    void calcRegionPos(Align align, int compW, int compH,
                       int regionW, int regionH,
                       int &outX, int &outY) const;
    void focusFirstFocusable();
    IComponent* focusedComponent() const;
    void moveFocus(int dx, int dy);

    Type type_;
    float r1_, r2_;
    Region regions_[4];
    bool hasFocus_;
    int focusRegion_;
    int focusComp_;
};

} // namespace ui
