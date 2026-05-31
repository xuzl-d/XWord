#pragma once

#include "Paragraph.hpp"
#include "Types.hpp"
#include <memory>
#include <string>
#include <vector>
#include <filesystem>

namespace xword {

/// Descriptor for an image placed inside a table cell.
struct CellImage {
    std::string filepath;
    int  width  = 0;
    int  height = 0;
    std::string rId;       ///< Relationship ID assigned during save.
    std::string caption;
    bool skipped = false;  ///< True if the source file was missing.

    CellImage& setCaption(const std::string& cap) { caption = cap; return *this; }
    CellImage& setCaption(const std::wstring& cap);
};

/// A single table cell.
///
/// Each cell may contain paragraphs (text, equations) and images.
/// Cells are accessed through Table::cell().
class Cell {
public:
    Cell();
    ~Cell();
    Cell(Cell&&) noexcept;
    Cell& operator=(Cell&&) noexcept;

    /// @{
    /// Add content to the cell (chainable).

    /// Add a plain paragraph.
    Paragraph& addParagraph(const std::string& text = "");
    Paragraph& addParagraph(const std::wstring& text);

    /// Add a paragraph with an initial styled run.
    Paragraph& addParagraph(const std::string& text, const RunStyle& style);
    Paragraph& addParagraph(const std::wstring& text, const RunStyle& style);

    /// Add a paragraph containing an inline equation.
    Paragraph& addEquation(const std::string& latex);
    Paragraph& addEquation(const std::wstring& latex);

    /// Add an image (returns CellImage& for optional caption chaining).
    CellImage& addImage(const std::string& filepath);
    CellImage& addImage(const std::string& filepath, int width, int height);

    /// @{
    /// Convenience overloads accepting various path types.
    CellImage& addImage(const char* filepath) { return addImage(std::string(filepath)); }
    CellImage& addImage(const char* filepath, int w, int h) { return addImage(std::string(filepath), w, h); }
    CellImage& addImage(const std::filesystem::path& filepath);
    CellImage& addImage(const std::filesystem::path& filepath, int width, int height);
    CellImage& addImage(const std::wstring& filepath);
    CellImage& addImage(const std::wstring& filepath, int width, int height);
    CellImage& addImage(const wchar_t* filepath) { return addImage(std::wstring(filepath)); }
    CellImage& addImage(const wchar_t* filepath, int w, int h) { return addImage(std::wstring(filepath), w, h); }
    /// @}
    /// @}

    /// @{
    /// Merge-related properties (set by Table::mergeCells).

    /// Vertical merge state: "restart" or "continue".
    void setVMerge(const std::string& v);
    /// Horizontal grid span (number of columns this cell occupies).
    void setGridSpan(int span);
    /// Mark cell as hidden (covered by a span).
    void setHidden(bool h);

    const std::string& vMerge()    const;
    int  gridSpan() const;
    bool hidden()   const;
    /// @}

    /// @{
    /// Read access to cell contents (internal use).
    const std::vector<std::unique_ptr<Paragraph>>& paragraphs() const;
    const std::vector<CellImage>& images() const;
    std::vector<CellImage>& images();
    /// @}

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

/// A table with a fixed number of rows and columns.
///
/// Created via Document::addTable(rows, cols).  Supports header rows,
/// cell merging, grid borders, and automatic caption numbering.
class Table {
public:
    Table(int rows, int cols);
    ~Table();
    Table(Table&&) noexcept;
    Table& operator=(Table&&) noexcept;

    /// @{
    /// Table properties (chainable).

    /// Designate a header row (repeated across page breaks).
    Table& setHeaderRow(int row);

    /// Border style preset.
    Table& setStyle(TableStyle style);

    /// Caption text shown above the table.
    Table& setCaption(const std::string& cap);
    Table& setCaption(const std::wstring& cap);
    const std::string& caption() const;
    /// @}

    /// @{
    /// Cell access (0-based indices).

    /// Mutable cell reference.
    Cell&       cell(int row, int col);
    /// Immutable cell reference.
    const Cell& cell(int row, int col) const;
    /// @}

    /// Merge a rectangular region of cells.
    /// @param row1, col1  Top-left corner (inclusive).
    /// @param row2, col2  Bottom-right corner (inclusive).
    Table& mergeCells(int row1, int col1, int row2, int col2);

    /// @name Dimensions
    /// @{
    int rows() const;
    int cols() const;
    /// @}

    /// @name Column width ratios
    /// @{
    /// Set width ratio for a single column (relative proportion; default is 1.0 for equal width).
    Table& setColumnWidth(int col, double ratio);
    /// Set width ratios for all columns at once.
    Table& setColumnWidths(const std::vector<double>& ratios);
    /// Get width ratio for a column.
    double getColumnWidth(int col) const;
    /// @}

    /// Build OOXML table XML (internal use).
    std::string toXml() const;

    /// Collect image pointers from all cells for registration (internal use).
    void collectCellImages(std::vector<CellImage*>& images);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace xword
