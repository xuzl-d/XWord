#include "xword/Table.hpp"
#include "internal/ZipWriter.hpp"

namespace xword {

// ---- Cell ----

struct Cell::Impl {
    std::vector<std::unique_ptr<Paragraph>> m_paragraphs;
    std::vector<CellImage> m_images;
    std::string m_vMerge;
    int  m_gridSpan = 0;
    bool m_hidden = false;
};

struct Table::Impl {
    int m_rows;
    int m_cols;
    int m_headerRow = -1;
    TableStyle m_style = TableStyle::None;
    std::string m_caption;
    std::vector<std::vector<Cell>> m_cells;
    std::vector<double> m_columnWidthRatios;  // Width ratio for each column (default 1.0)
    bool m_hasCustomWidths = false;            // true when user explicitly sets widths
};

Cell::Cell() : m_impl(std::make_unique<Impl>()) {}
Cell::~Cell() = default;
Cell::Cell(Cell&&) noexcept = default;
Cell& Cell::operator=(Cell&&) noexcept = default;

Paragraph& Cell::addParagraph(const std::string& text) {
    auto p = std::make_unique<Paragraph>();
    if (!text.empty()) p->addRun(text);
    p->setFirstLineIndent(0);
    p->setSpacingAfter(0);
    m_impl->m_paragraphs.push_back(std::move(p));
    return *m_impl->m_paragraphs.back();
}

Paragraph& Cell::addParagraph(const std::string& text, const RunStyle& style) {
    auto p = std::make_unique<Paragraph>();
    if (!text.empty()) p->addRun(text, style);
    p->setFirstLineIndent(0);
    p->setSpacingAfter(0);
    m_impl->m_paragraphs.push_back(std::move(p));
    return *m_impl->m_paragraphs.back();
}

Paragraph& Cell::addEquation(const std::string& latex) {
    auto p = std::make_unique<Paragraph>();
    p->addEquation(latex);
    p->setFirstLineIndent(0);
    p->setSpacingAfter(0);
    m_impl->m_paragraphs.push_back(std::move(p));
    return *m_impl->m_paragraphs.back();
}

CellImage& Cell::addImage(const std::string& filepath) {
    return addImage(filepath, 0, 0);
}

CellImage& Cell::addImage(const std::string& filepath, int width, int height) {
    m_impl->m_images.push_back({filepath, width, height, ""});
    return m_impl->m_images.back();
}

CellImage& Cell::addImage(const std::filesystem::path& filepath) {
    return addImage(filepath.u8string(), 0, 0);
}

CellImage& Cell::addImage(const std::filesystem::path& filepath, int width, int height) {
    return addImage(filepath.u8string(), width, height);
}

CellImage& Cell::addImage(const std::wstring& filepath) {
    return addImage(std::filesystem::path(filepath));
}

CellImage& Cell::addImage(const std::wstring& filepath, int width, int height) {
    return addImage(std::filesystem::path(filepath), width, height);
}

void Cell::setVMerge(const std::string& v) { m_impl->m_vMerge = v; }
void Cell::setGridSpan(int span)           { m_impl->m_gridSpan = span; }
void Cell::setHidden(bool h)               { m_impl->m_hidden = h; }
const std::string& Cell::vMerge() const    { return m_impl->m_vMerge; }
int  Cell::gridSpan() const                { return m_impl->m_gridSpan; }
bool Cell::hidden() const                  { return m_impl->m_hidden; }

const std::vector<std::unique_ptr<Paragraph>>& Cell::paragraphs() const { return m_impl->m_paragraphs; }
const std::vector<CellImage>& Cell::images() const { return m_impl->m_images; }
std::vector<CellImage>& Cell::images()             { return m_impl->m_images; }

// ---- Table ----

Table::Table(int rows, int cols)
    : m_impl(std::make_unique<Impl>())
{
    m_impl->m_rows = rows;
    m_impl->m_cols = cols;
    m_impl->m_cells.resize(rows);
    for (int r = 0; r < rows; ++r)
        m_impl->m_cells[r].resize(cols);
    // Initialize column width ratios with default value 1.0
    m_impl->m_columnWidthRatios.resize(cols, 1.0);
}

Table::~Table() = default;
Table::Table(Table&&) noexcept = default;
Table& Table::operator=(Table&&) noexcept = default;

Table& Table::setHeaderRow(int row) { m_impl->m_headerRow = row; return *this; }
Table& Table::setStyle(TableStyle style) { m_impl->m_style = style; return *this; }
Table& Table::setCaption(const std::string& cap) { m_impl->m_caption = cap; return *this; }
const std::string& Table::caption() const { return m_impl->m_caption; }

Cell& Table::cell(int row, int col) { return m_impl->m_cells[row][col]; }
const Cell& Table::cell(int row, int col) const { return m_impl->m_cells[row][col]; }

int Table::rows() const { return m_impl->m_rows; }
int Table::cols() const { return m_impl->m_cols; }

Table& Table::setColumnWidth(int col, double ratio) {
    if (col >= 0 && col < m_impl->m_cols) {
        m_impl->m_columnWidthRatios[col] = ratio;
        m_impl->m_hasCustomWidths = true;
    }
    return *this;
}

Table& Table::setColumnWidths(const std::vector<double>& ratios) {
    if (ratios.size() == static_cast<size_t>(m_impl->m_cols)) {
        m_impl->m_columnWidthRatios = ratios;
        m_impl->m_hasCustomWidths = true;
    }
    return *this;
}

double Table::getColumnWidth(int col) const {
    if (col >= 0 && col < m_impl->m_cols)
        return m_impl->m_columnWidthRatios[col];
    return 1.0;
}

Table& Table::mergeCells(int row1, int col1, int row2, int col2) {
    int spanW = col2 - col1 + 1;
    if (row1 == row2 && col1 == col2) return *this;

    if (row1 == row2) {
        m_impl->m_cells[row1][col1].setGridSpan(spanW);
        for (int c = col1 + 1; c <= col2; ++c)
            m_impl->m_cells[row1][c].setHidden(true);
    } else if (col1 == col2) {
        m_impl->m_cells[row1][col1].setVMerge("restart");
        for (int r = row1 + 1; r <= row2; ++r)
            m_impl->m_cells[r][col1].setVMerge("continue");
    } else {
        m_impl->m_cells[row1][col1].setGridSpan(spanW);
        m_impl->m_cells[row1][col1].setVMerge("restart");
        for (int c = col1 + 1; c <= col2; ++c)
            m_impl->m_cells[row1][c].setHidden(true);
        for (int r = row1 + 1; r <= row2; ++r) {
            m_impl->m_cells[r][col1].setVMerge("continue");
            for (int c = col1 + 1; c <= col2; ++c)
                m_impl->m_cells[r][c].setHidden(true);
        }
    }
    return *this;
}

void Table::collectCellImages(std::vector<CellImage*>& images) {
    for (int r = 0; r < m_impl->m_rows; ++r) {
        for (int c = 0; c < m_impl->m_cols; ++c) {
            if (m_impl->m_cells[r][c].hidden()) continue;
            for (auto& img : m_impl->m_cells[r][c].images())
                images.push_back(&img);
        }
    }
}

std::string Table::toXml() const {
    using namespace internal;

    auto buildImageXml = [](const CellImage& img) -> std::string {
        auto sz = internal::computeImageSize(img.filepath, img.width, img.height);
        std::string cx = std::to_string(sz.widthEmu);
        std::string cy = std::to_string(sz.heightEmu);
        std::string idStr = img.rId.empty() ? "1"
            : std::to_string(std::hash<std::string>{}(img.rId) % 1000000 + 1);
        return "<w:r>"
               "<w:drawing>"
               "<wp:inline distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\">"
               "<wp:extent cx=\"" + cx + "\" cy=\"" + cy + "\"/>"
               "<wp:effectExtent l=\"0\" t=\"0\" r=\"0\" b=\"0\"/>"
               "<wp:docPr id=\"" + idStr + "\" name=\"Picture " + idStr + "\"/>"
               "<wp:cNvGraphicFramePr><a:graphicFrameLocks xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" noChangeAspect=\"1\"/></wp:cNvGraphicFramePr>"
               "<a:graphic xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\">"
               "<a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/picture\">"
               "<pic:pic xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\">"
               "<pic:nvPicPr>"
               "<pic:cNvPr id=\"" + idStr + "\" name=\"Picture " + idStr + "\"/>"
               "<pic:cNvPicPr/>"
               "</pic:nvPicPr>"
               "<pic:blipFill>"
               "<a:blip r:embed=\"" + img.rId + "\"/>"
               "<a:stretch><a:fillRect/></a:stretch>"
               "</pic:blipFill>"
               "<pic:spPr>"
               "<a:xfrm>"
               "<a:off x=\"0\" y=\"0\"/>"
               "<a:ext cx=\"" + cx + "\" cy=\"" + cy + "\"/>"
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

    xml += "<w:tblPr>";
    if (m_impl->m_style == TableStyle::Grid) {
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
    xml += "<w:jc w:val=\"center\"/>";
    if (m_impl->m_hasCustomWidths)
        xml += "<w:tblLayout w:type=\"fixed\"/>";
    xml += "</w:tblPr>";

    xml += "<w:tblGrid>";
    // Calculate total width ratio and individual column widths
    double totalRatio = 0.0;
    for (int c = 0; c < m_impl->m_cols; ++c)
        totalRatio += m_impl->m_columnWidthRatios[c];
    
    for (int c = 0; c < m_impl->m_cols; ++c) {
        double ratio = m_impl->m_columnWidthRatios[c];
        int width = static_cast<int>(9500.0 * ratio / totalRatio);
        xml += "<w:gridCol w:w=\"" + std::to_string(width) + "\"/>";
    }
    xml += "</w:tblGrid>";

    for (int r = 0; r < m_impl->m_rows; ++r) {
        xml += "<w:tr>";
        if (m_impl->m_headerRow == r)
            xml += "<w:trPr><w:tblHeader/></w:trPr>";

        for (int c = 0; c < m_impl->m_cols; ++c) {
            const Cell& cell = m_impl->m_cells[r][c];
            if (cell.hidden()) continue;

            xml += "<w:tc><w:tcPr>";
            if (cell.gridSpan() > 1)
                xml += "<w:gridSpan w:val=\"" + std::to_string(cell.gridSpan()) + "\"/>";
            if (!cell.vMerge().empty())
                xml += "<w:vMerge w:val=\"" + cell.vMerge() + "\"/>";
            xml += "</w:tcPr>";

            int liveImages = 0;
            for (const auto& img : cell.images()) {
                if (img.skipped) continue;
                ++liveImages;
                xml += "<w:p>" + buildImageXml(img) + "</w:p>";
                if (!img.caption.empty()) {
                    xml += "<w:p><w:pPr><w:jc w:val=\"center\"/>"
                           "<w:r><w:rPr><w:b/></w:rPr>"
                           "<w:t xml:space=\"preserve\">" + internal::xmlEscape(img.caption) + "</w:t>"
                           "</w:r></w:p>";
                }
            }
            for (const auto& p : cell.paragraphs())
                xml += p->toXml();

            if (cell.paragraphs().empty() && liveImages == 0)
                xml += "<w:p/>";

            xml += "</w:tc>";
        }
        xml += "</w:tr>";
    }

    xml += "</w:tbl>";
    return xml;
}

} // namespace xword
