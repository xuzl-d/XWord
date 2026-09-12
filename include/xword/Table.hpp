#pragma once

#include <vector>

#include "Content.hpp"

namespace xword
{
using CellImage = Image;

/// A single table cell.  Cells are content containers, so they accept
/// paragraphs, equations, images, and nested tables:
///
///     table.cell(0, 0).addParagraph("Value").setAlignment(Alignment::Center);
class Cell : public Content
{
public:
    /// Create an empty cell.
    Cell();

    /// @name Rule of five
    /// A cell owns its content: movable, not copyable.
    /// @{
    ~Cell();
    Cell(Cell&&) noexcept;
    Cell& operator=(Cell&&) noexcept;
    /// @}

    /// Vertical alignment of the cell's content.
    Cell& setVAlign(VAlignment v);

    /// Current vertical alignment.
    VAlignment vAlign() const;

    /// Background fill colour as hex RRGGBB.
    Cell& setShading(const std::string& color);

    /// Cell border, overriding the table's border.
    Cell& setBorder(const Border& border);

    /// Cell padding (w:tcMar).
    Cell& setMargins(Length top, Length bottom, Length left, Length right);

    /// @{
    /// Vertical merge control, used internally by Table::mergeCells().
    /// @param v  "restart" to open a merged range, "continue" to extend it,
    ///           empty to clear.
    void               setVMerge(const std::string& v);
    const std::string& vMerge() const;
    /// @}

    /// @{
    /// Horizontal merge control, used internally by Table::mergeCells().
    /// @param n  Number of grid columns this cell spans (>= 1).
    void setGridSpan(int n);
    int  gridSpan() const;
    /// @}

    /// @{
    /// Cells swallowed by a merge are hidden and emit no `<w:tc>`.
    void setHidden(bool on);
    bool hidden() const;
    /// @}

    /// Build the `<w:tcPr>` fragment (internal use).
    std::string propertiesXml() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

/// A grid-border table.
///
/// Tables are created through Document::addTable(), which also applies the
/// document's table run style.  Cells are addressed by row/column index:
///
///     auto& t = doc.addTable(2, 3);
///     t.setBorderStyle(TableStyle::Grid).setCaption("Results");
///     t.cell(0, 0).addParagraph("Header");
class Table
{
public:
    /// @param rows  Row count.
    /// @param cols  Column count, 1..63.
    Table(int rows, int cols);

    /// @name Rule of five
    /// A table owns its cells: movable, not copyable.
    /// @{
    ~Table();
    Table(Table&&) noexcept;
    Table& operator=(Table&&) noexcept;
    /// @}

    /// Repeat the given row as a header row on every page.
    /// @param lastRow  Zero-based index of the last row to repeat.
    Table& setHeaderRow(int lastRow);

    /// Repeat the first @p count rows as header rows on every page.
    Table& setHeaderRows(int count);

    /// Allow or forbid a row being split across pages.
    Table& setRowAllowSplit(int row, bool allow = true);

    /// Set the row height.
    /// @param exact  True for an exact height, false for a minimum.
    Table& setRowHeight(int row, Length height, bool exact = false);

    /// Set the total table width.  Unset means the full text width.
    Table& setWidth(Length width);

    /// Auto-fit columns to their content; false uses fixed column widths.
    Table& setAutoFit(bool on = true);

    /// Apply a table style by style ID, e.g. one registered with
    /// Document::registerTableStyle().
    Table& setStyleId(const std::string& id);

    /// Apply one of the built-in border presets.
    Table& setBorderStyle(TableStyle style);

    /// Apply an explicit border definition.
    Table& setBorder(const Border& border);

    /// Background fill colour as hex RRGGBB.
    Table& setShading(const std::string& color);

    /// Default cell padding for every cell (w:tblCellMar).
    Table& setCellMargins(Length top, Length bottom, Length left, Length right);

    /// Run style inherited by every cell's content.
    Table& setStyle(const RunStyle& style);

    /// The style set by setStyle() (mutable for chaining).
    RunStyle& getStyle();

    /// Apply a vertical alignment to every cell.
    Table& setVAlign(VAlignment v);

    /// Caption drawn above the table.  When the document has table numbering
    /// enabled, the sequence number is inserted before this text.
    Table& setCaption(const std::string& text);
    Table& setCaption(const std::wstring& text);

    /// Bookmark this table so it can be referenced and numbered.
    Table& setBookmark(const TargetId& target);

    /// The caption text set by setCaption().
    const std::string& caption() const;

    /// @{
    /// Access a cell.  Indices are zero-based and bounds-checked.
    Cell&       cell(int row, int col);
    const Cell& cell(int row, int col) const;
    /// @}

    /// Merge a rectangular range into one cell.
    /// @param row1,col1  Top-left cell.
    /// @param row2,col2  Bottom-right cell.
    /// @throws std::invalid_argument if the range is reversed or overlaps an
    ///         existing merge.
    Table& mergeCells(int row1, int col1, int row2, int col2);

    /// Row count.
    int rows() const;

    /// Column count.
    int cols() const;

    /// Give one column a relative width.  Calling this switches the table to
    /// fixed layout.
    Table& setColumnWidth(int col, double ratio);

    /// Give every column a relative width.
    /// @throws std::invalid_argument if the count differs from cols().
    Table& setColumnWidths(const std::vector<double>& ratios);

    /// The relative width set for a column.
    double getColumnWidth(int col) const;

    /// Build the OOXML for this table (internal use).
    std::string toXml() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};
} // namespace xword
