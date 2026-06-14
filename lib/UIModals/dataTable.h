#ifndef DATATABLE_H
#define DATATABLE_H

#include <string>
#include <vector>
#include <ostream>
#include <functional>
#include "UIControllers/_component.h"

namespace ui {

struct TableRow {
    std::vector<std::string> columns;
    TableRow() = default;
    TableRow(const std::vector<std::string> &cols) : columns(cols) {}
};

class DataTable : public ComponentBase {
public:
    DataTable(
        int width = 50,
        int height = 10,
        const std::string &bgColor = "#0d1117",
        const std::string &fgColor = "#c9d1d9",
        const std::string &borderColor = "#30363d",
        const std::string &headerBgColor = "#161b22",
        const std::string &headerFgColor = "#58a6ff",
        const std::string &altRowColor = "#161b22",
        int maxRows = 5
    );

    template<typename... NPs,
             typename = std::enable_if_t<(is_param_v<std::decay_t<NPs>> || ...)>>
    DataTable(NPs&&... params)
        : ComponentBase(50, 10)
        , bgColor_("#0d1117")
        , fgColor_("#c9d1d9")
        , borderColor_("#30363d")
        , headerBgColor_("#161b22")
        , headerFgColor_("#58a6ff")
        , altRowColor_("#161b22")
        , maxRows_(5)
    {
        align_ = Align::TopLeft;
        (apply_param(std::forward<NPs>(params)), ...);
    }

    void setHeaders(const std::vector<std::string> &headers) { headers_ = headers; setDirty(); }
    const std::vector<std::string>& headers() const { return headers_; }
    void setData(const std::vector<TableRow> &rows) { rows_ = rows; setDirty(); }
    void addRow(const TableRow &row);
    void setMaxRows(int max) { maxRows_ = max; setDirty(); }
    int currentPage() const { return currentPage_; }
    int totalPages() const;
    void nextPage();
    void prevPage();
    void resetPage() { currentPage_ = 0; setDirty(); }

    std::string toString() const override;

    bool isFocusable() const override { return true; }
    void setSelected(bool s) override { if (s != selected_) { selected_ = s; setDirty(); } }
    bool selected() const override { return selected_; }
    bool handleKey(int key) override;
    void onActivate() override { if (onActivate_) onActivate_(); }
    void setOnActivate(std::function<void()> cb) override { onActivate_ = cb; }

private:
    using ComponentBase::apply_param;
    void apply_param(Param<std::string, bgColor_tag> p) { bgColor_ = p.value; }
    void apply_param(Param<std::string, fgColor_tag> p) { fgColor_ = p.value; }
    void apply_param(Param<std::string, borderColor_tag> p) { borderColor_ = p.value; }
    void apply_param(Param<std::string, titleBgColor_tag> p) { headerBgColor_ = p.value; }
    void apply_param(Param<std::string, titleFgColor_tag> p) { headerFgColor_ = p.value; }
    void apply_param(Param<std::string, bodyBgColor_tag> p) { altRowColor_ = p.value; }
    void apply_param(Param<int, width_tag> p) { width_ = p.value; }
    void apply_param(Param<int, height_tag> p) { height_ = p.value; }
    void apply_param(Param<int, maxLength_tag> p) { maxRows_ = p.value; }

    std::vector<int> computeColumnWidths(int availWidth) const;

    bool selected_ = false;
    int selectedRow_ = 0;
    std::function<void()> onActivate_;

    std::string bgColor_;
    std::string fgColor_;
    std::string borderColor_;
    std::string headerBgColor_;
    std::string headerFgColor_;
    std::string altRowColor_;
    std::vector<std::string> headers_;
    std::vector<TableRow> rows_;
    int maxRows_ = 5;
    mutable int currentPage_ = 0;
};

} // namespace ui

#endif // DATATABLE_H
