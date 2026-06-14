#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <string>
#include <vector>
#include <ostream>
#include "UIControllers/_component.h"

namespace ui {

struct HistogramBar {
    std::string label;
    float value;
    std::string color;
    HistogramBar(const std::string &l = "", float v = 0, const std::string &c = "#4a9eff")
        : label(l), value(v), color(c) {}
};

struct GridCell {
    float value;
    std::string color;
    std::string tooltip;
    GridCell(float v = 0, const std::string &c = "#161b22", const std::string &t = "")
        : value(v), color(c), tooltip(t) {}
};

struct GridRow {
    std::string label;
    std::vector<GridCell> cells;
    GridRow(const std::string &l = "") : label(l) {}
};

class Histogram : public ComponentBase {
public:
    Histogram(
        const std::string &title = "Histogram",
        int width = 40,
        int height = 10,
        const std::string &bgColor = "#1a1a2e",
        const std::string &fgColor = "#ffffff",
        const std::string &borderColor = "#444444",
        const std::string &titleBgColor = "#16213e",
        const std::string &titleFgColor = "#4a9eff"
    );

    template<typename... NPs,
             typename = std::enable_if_t<(is_param_v<std::decay_t<NPs>> || ...)>>
    Histogram(NPs&&... params)
        : ComponentBase(40, 10)
        , title_("Histogram")
        , bgColor_("#1a1a2e")
        , fgColor_("#ffffff")
        , borderColor_("#444444")
        , titleBgColor_("#16213e")
        , titleFgColor_("#4a9eff")
    {
        align_ = Align::TopLeft;
        (apply_param(std::forward<NPs>(params)), ...);
    }

    void setData(const std::vector<HistogramBar> &bars) { bars_ = bars; setDirty(); }
    void addBar(const std::string &label, float value, const std::string &color = "#4a9eff");
    void clearBars() { bars_.clear(); gridRows_.clear(); setDirty(); }
    int barCount() const { return static_cast<int>(bars_.size()); }
    void setTitle(const std::string &title) { title_ = title; setDirty(); }
    void setMaxValue(float max) { maxValue_ = max; setDirty(); }
    void setShowValues(bool show) { showValues_ = show; setDirty(); }

    enum ChartMode { Bar, Pie, Grid };
    void setChartMode(ChartMode mode) { chartMode_ = mode; setDirty(); }
    ChartMode chartMode() const { return chartMode_; }

    void setGridData(const std::vector<GridRow> &rows) { gridRows_ = rows; setDirty(); }
    void addGridRow(const GridRow &row) { gridRows_.push_back(row); setDirty(); }
    void clearGrid() { gridRows_.clear(); setDirty(); }

    std::string toString() const override;

private:
    using ComponentBase::apply_param;
    void apply_param(Param<std::string, title_tag> p) { title_ = p.value; }
    void apply_param(Param<std::string, bgColor_tag> p) { bgColor_ = p.value; }
    void apply_param(Param<std::string, fgColor_tag> p) { fgColor_ = p.value; }
    void apply_param(Param<std::string, borderColor_tag> p) { borderColor_ = p.value; }
    void apply_param(Param<std::string, titleBgColor_tag> p) { titleBgColor_ = p.value; }
    void apply_param(Param<std::string, titleFgColor_tag> p) { titleFgColor_ = p.value; }
    void apply_param(Param<int, width_tag> p) { width_ = p.value; }
    void apply_param(Param<int, height_tag> p) { height_ = p.value; }

    std::string title_;
    std::string bgColor_;
    std::string fgColor_;
    std::string borderColor_;
    std::string titleBgColor_;
    std::string titleFgColor_;
    std::vector<HistogramBar> bars_;
    mutable float maxValue_ = 0;
    bool showValues_ = true;
    ChartMode chartMode_ = Bar;
    std::vector<GridRow> gridRows_;

    float computeMax() const;
    std::string renderBarMode(int w, int bgR, int bgG, int bgB, int fgR, int fgG, int fgB, int bR, int bG, int bB) const;
    std::string renderPieMode(int w, int bgR, int bgG, int bgB, int fgR, int fgG, int fgB, int bR, int bG, int bB) const;
    std::string renderGridMode(int w, int bgR, int bgG, int bgB, int fgR, int fgG, int fgB, int bR, int bG, int bB) const;
    std::string renderTitle(int w, int bR, int bG, int bB, int tR, int tG, int tB, int tFgR, int tFgG, int tFgB) const;
    std::string renderBottom(int w, int bR, int bG, int bB) const;
    std::string renderEmptyRow(int w, int bgR, int bgG, int bgB, int fgR, int fgG, int fgB, int bR, int bG, int bB) const;
};

} // namespace ui

#endif // HISTOGRAM_H
