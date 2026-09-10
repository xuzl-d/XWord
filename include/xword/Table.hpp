#pragma once
#include "Content.hpp"
#include <vector>
namespace xword {
using CellImage = Image;
class Cell : public Content {
public:
    Cell(); ~Cell();
    Cell(Cell&&) noexcept; Cell& operator=(Cell&&) noexcept;
    Cell& setVAlign(VAlignment v);
    VAlignment vAlign() const;
    Cell& setShading(const std::string& color);
    Cell& setBorder(const Border& border);
    Cell& setMargins(Length top,Length bottom,Length left,Length right);
    void setVMerge(const std::string& v);
    void setGridSpan(int n);
    void setHidden(bool on);
    const std::string& vMerge() const;
    int gridSpan() const;
    bool hidden() const;
    std::string propertiesXml() const;
private:
    struct Impl; std::unique_ptr<Impl> m_impl;
};
class Table {
public:
    Table(int rows,int cols); ~Table();
    Table(Table&&) noexcept; Table& operator=(Table&&) noexcept;
    Table& setHeaderRow(int lastRow);
    Table& setHeaderRows(int count);
    Table& setRowAllowSplit(int row,bool allow = true);
    Table& setRowHeight(int row,Length height,bool exact = false);
    Table& setWidth(Length width);
    Table& setAutoFit(bool on = true);
    Table& setStyleId(const std::string& id);
    Table& setBorderStyle(TableStyle style);
    Table& setBorder(const Border& border);
    Table& setShading(const std::string& color);
    Table& setCellMargins(Length top,Length bottom,Length left,Length right);
    Table& setStyle(const RunStyle& style);
    RunStyle& getStyle();
    Table& setVAlign(VAlignment v);
    Table& setCaption(const std::string& text);
    Table& setCaption(const std::wstring& text);
    Table& setBookmark(const TargetId& target);
    const std::string& caption() const;
    Cell& cell(int row,int col);
    const Cell& cell(int row,int col) const;
    Table& mergeCells(int row1,int col1,int row2,int col2);
    int rows() const; int cols() const;
    Table& setColumnWidth(int col,double ratio);
    Table& setColumnWidths(const std::vector<double>& ratios);
    double getColumnWidth(int col) const;
    std::string toXml() const;
private:
    struct Impl; std::unique_ptr<Impl> m_impl;
};
}
