#pragma once

#include "Paragraph.hpp"
#include "Types.hpp"
#include <string>
#include <vector>
#include <memory>

namespace xword {

struct CellImage {
    std::string filepath;
    int width = 0;
    int height = 0;
    std::string rId;      // assigned by Document during save
    std::string caption;

    CellImage& setCaption(const std::string& cap) { caption = cap; return *this; }
};

class Cell {
public:
    Cell() = default;

    Paragraph& addParagraph(const std::string& text = "");

    // Convenience: add inline equation directly
    Paragraph& addEquation(const std::string& latex);

    // Add an image to this cell
    CellImage& addImage(const std::string& filepath);
    CellImage& addImage(const std::string& filepath, int width, int height);

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
    std::vector<std::vector<Cell>> m_cells;
};

} // namespace xword
