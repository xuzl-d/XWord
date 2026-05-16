#pragma once

#include "Paragraph.hpp"
#include "Types.hpp"
#include <string>
#include <vector>
#include <memory>
#include <filesystem>

namespace xword {

struct CellImage {
    std::string filepath;
    int width = 0;
    int height = 0;
    std::string rId;      // assigned by Document during save
    std::string caption;
    bool skipped = false; // true when source file is missing/unreadable

    CellImage& setCaption(const std::string& cap) { caption = cap; return *this; }
};

class Cell {
public:
    Cell() = default;

    Paragraph& addParagraph(const std::string& text = "");

    // Convenience: add inline equation directly
    Paragraph& addEquation(const std::string& latex);

    // Add an image to this cell.
    // std::string is interpreted as UTF-8; use fs::path or std::wstring for native paths.
    CellImage& addImage(const std::string& filepath);
    CellImage& addImage(const std::string& filepath, int width, int height);
    CellImage& addImage(const char* filepath) { return addImage(std::string(filepath)); }
    CellImage& addImage(const char* filepath, int width, int height) { return addImage(std::string(filepath), width, height); }
    CellImage& addImage(const std::filesystem::path& filepath);
    CellImage& addImage(const std::filesystem::path& filepath, int width, int height);
    CellImage& addImage(const std::wstring& filepath);
    CellImage& addImage(const std::wstring& filepath, int width, int height);
    CellImage& addImage(const wchar_t* filepath) { return addImage(std::wstring(filepath)); }
    CellImage& addImage(const wchar_t* filepath, int width, int height) { return addImage(std::wstring(filepath), width, height); }

    void setVMerge(const std::string& v) { m_vMerge = v; }
    void setGridSpan(int span) { m_gridSpan = span; }
    void setHidden(bool h) { m_hidden = h; }
    const std::string& vMerge() const { return m_vMerge; }
    int gridSpan() const { return m_gridSpan; }
    bool hidden() const { return m_hidden; }

    const std::vector<std::unique_ptr<Paragraph>>& paragraphs() const { return m_paragraphs; }
    const std::vector<CellImage>& images() const { return m_images; }
    std::vector<CellImage>& images() { return m_images; }

private:
    std::vector<std::unique_ptr<Paragraph>> m_paragraphs;
    std::vector<CellImage> m_images;
    std::string m_vMerge;
    int m_gridSpan = 0;
    bool m_hidden = false;
};

class Table {
public:
    Table(int rows, int cols);

    Table& setHeaderRow(int row);
    Table& setStyle(TableStyle style);
    Table& setCaption(const std::string& cap) { m_caption = cap; return *this; }
    const std::string& caption() const { return m_caption; }

    Cell& cell(int row, int col);
    const Cell& cell(int row, int col) const;

    Table& mergeCells(int row1, int col1, int row2, int col2);

    int rows() const { return m_rows; }
    int cols() const { return m_cols; }

    // Internal
    std::string toXml() const;

    // Collect all cell images for registration
    void collectCellImages(std::vector<CellImage*>& images);

private:
    int m_rows;
    int m_cols;
    int m_headerRow = -1;
    TableStyle m_style = TableStyle::None;
    std::string m_caption;
    std::vector<std::vector<Cell>> m_cells;
};

} // namespace xword
