#include "xword/Table.hpp"
#include "xword/internal/ZipWriter.hpp"

namespace xword {

// ---- Cell ----

Paragraph& Cell::addParagraph(const std::string& text) {
    auto p = std::make_unique<Paragraph>();
    if (!text.empty()) {
        p->addRun(text);
    }
    m_paragraphs.push_back(std::move(p));
    return *m_paragraphs.back();
}

Paragraph& Cell::addEquation(const std::string& latex) {
    auto p = std::make_unique<Paragraph>();
    p->addEquation(latex);
    m_paragraphs.push_back(std::move(p));
    return *m_paragraphs.back();
}

CellImage& Cell::addImage(const std::string& filepath) {
    return addImage(filepath, 0, 0);
}

CellImage& Cell::addImage(const std::string& filepath, int width, int height) {
    m_images.push_back({filepath, width, height, ""});
    return m_images.back();
}

// ---- Table ----

Table::Table(int rows, int cols) : m_rows(rows), m_cols(cols) {
    m_cells.resize(rows);
    for (int r = 0; r < rows; ++r) {
        m_cells[r].resize(cols);
    }
}

Table& Table::setHeaderRow(int row) {
    m_headerRow = row;
    return *this;
}

Table& Table::setStyle(TableStyle style) {
    m_style = style;
    return *this;
}

Cell& Table::cell(int row, int col) {
    return m_cells[row][col];
}

const Cell& Table::cell(int row, int col) const {
    return m_cells[row][col];
}

Table& Table::mergeCells(int row1, int col1, int row2, int col2) {
    int spanW = col2 - col1 + 1;

    if (row1 == row2 && col1 == col2) {
        return *this;  // nothing to merge
    }

    if (row1 == row2) {
        // ---- Horizontal merge ----
        m_cells[row1][col1].setGridSpan(spanW);
        // Hide the right-side cells covered by gridSpan
        for (int c = col1 + 1; c <= col2; ++c) {
            m_cells[row1][c].setHidden(true);
        }
    } else if (col1 == col2) {
        // ---- Vertical merge ----
        m_cells[row1][col1].setVMerge("restart");
        for (int r = row1 + 1; r <= row2; ++r) {
            m_cells[r][col1].setVMerge("continue");
        }
    } else {
        // ---- Rectangle merge ----
        // Anchor cell spans horizontally + starts vertical merge
        m_cells[row1][col1].setGridSpan(spanW);
        m_cells[row1][col1].setVMerge("restart");

        // Cells in first row covered by gridSpan → hidden
        for (int c = col1 + 1; c <= col2; ++c) {
            m_cells[row1][c].setHidden(true);
        }

        // Remaining rows
        for (int r = row1 + 1; r <= row2; ++r) {
            // First column of merge area: continue vertical merge
            m_cells[r][col1].setVMerge("continue");
            // Remaining cells in merge area → hidden
            for (int c = col1 + 1; c <= col2; ++c) {
                m_cells[r][c].setHidden(true);
            }
        }
    }
    return *this;
}

void Table::collectCellImages(std::vector<CellImage*>& images) {
    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            if (m_cells[r][c].hidden()) continue;
            for (auto& img : m_cells[r][c].images()) {
                images.push_back(&img);
            }
        }
    }
}

std::string Table::toXml() const {
    using namespace internal;

    auto buildImageXml = [](const CellImage& img) -> std::string {
        int wEmu = img.width > 0 ? img.width * 9525 : 1905000;   // default 200px
        int hEmu = img.height > 0 ? img.height * 9525 : 1428750;  // default 150px
        return "<w:r>"
               "<w:drawing>"
               "<wp:inline distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\">"
               "<wp:extent cx=\"" + std::to_string(wEmu) + "\" cy=\"" + std::to_string(hEmu) + "\"/>"
               "<wp:docPr id=\"1\" name=\"Picture\"/>"
               "<a:graphic xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\">"
               "<a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/picture\">"
               "<pic:pic xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\">"
               "<pic:nvPicPr>"
               "<pic:cNvPr id=\"0\" name=\"Picture\"/>"
               "<pic:cNvPicPr/>"
               "</pic:nvPicPr>"
               "<pic:blipFill>"
               "<a:blip r:embed=\"" + img.rId + "\"/>"
               "<a:stretch><a:fillRect/></a:stretch>"
               "</pic:blipFill>"
               "<pic:spPr>"
               "<a:xfrm>"
               "<a:off x=\"0\" y=\"0\"/>"
               "<a:ext cx=\"" + std::to_string(wEmu) + "\" cy=\"" + std::to_string(hEmu) + "\"/>"
               "</a:xfrm>"
               "<a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom>"
               "</pic:spPr>"
               "</pic:pic>"
               "</a:graphicData>"
               "</a:graphic>"
               "</wp:inline>"
               "</w:drawing>"
               "</w:r>";
    };

    std::string xml;
    xml += "<w:tbl>";

    // Table properties
    xml += "<w:tblPr>";
    if (m_style == TableStyle::Grid) {
        xml += "<w:tblBorders>"
               "<w:top w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/>"
               "<w:left w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/>"
               "<w:bottom w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/>"
               "<w:right w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/>"
               "<w:insideH w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/>"
               "<w:insideV w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/>"
               "</w:tblBorders>";
    }
    xml += "<w:tblW w:w=\"5000\" w:type=\"pct\"/>";
    xml += "</w:tblPr>";

    // Table grid
    xml += "<w:tblGrid>";
    for (int c = 0; c < m_cols; ++c) {
        xml += "<w:gridCol w:w=\"" + std::to_string(9500 / m_cols) + "\"/>";
    }
    xml += "</w:tblGrid>";

    // Table rows
    for (int r = 0; r < m_rows; ++r) {
        xml += "<w:tr>";
        if (m_headerRow == r) {
            xml += "<w:trPr><w:tblHeader/></w:trPr>";
        }
        for (int c = 0; c < m_cols; ++c) {
            const Cell& cell = m_cells[r][c];
            if (cell.hidden()) continue; // skip cells covered by gridSpan
            xml += "<w:tc>";
            // Cell properties
            xml += "<w:tcPr>";
            if (cell.gridSpan() > 1) {
                xml += "<w:gridSpan w:val=\"" + std::to_string(cell.gridSpan()) + "\"/>";
            }
            if (!cell.vMerge().empty()) {
                xml += "<w:vMerge w:val=\"" + cell.vMerge() + "\"/>";
            }
            xml += "</w:tcPr>";

            // Cell content: images first, then paragraphs
            for (const auto& img : cell.images()) {
                xml += "<w:p>" + buildImageXml(img) + "</w:p>";
                // Caption below image
                if (!img.caption.empty()) {
                    xml += "<w:p>"
                           "<w:pPr><w:jc w:val=\"center\"/>"
                           "<w:r><w:rPr><w:b/></w:rPr>"
                           "<w:t xml:space=\"preserve\">" + internal::xmlEscape(img.caption) + "</w:t>"
                           "</w:r>"
                           "</w:p>";
                }
            }
            for (const auto& p : cell.paragraphs()) {
                xml += p->toXml();
            }
            // Empty cell - add empty paragraph
            if (cell.paragraphs().empty() && cell.images().empty()) {
                xml += "<w:p/>";
            }
            xml += "</w:tc>";
        }
        xml += "</w:tr>";
    }

    xml += "</w:tbl>";
    return xml;
}

} // namespace xword
