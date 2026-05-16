#include "xword/Document.hpp"
#include "xword/internal/ZipWriter.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <filesystem>

namespace xword {

using namespace internal;

// ---- BulletList ----

BulletList::BulletList(ListType type) : m_type(type) {}

BulletList& BulletList::addItem(const std::string& text) {
    m_items.push_back(text);
    return *this;
}

BulletList& BulletList::setLevel(int level) {
    m_level = level;
    return *this;
}

BulletList& BulletList::setNumId(int id) {
    m_numId = id;
    return *this;
}

std::string BulletList::toXml() const {
    std::string xml;
    int numId = m_numId > 0 ? m_numId : ((m_type == ListType::Bullet) ? 1 : 2);
    for (const auto& item : m_items) {
        xml += "<w:p>"
               "<w:pPr>"
               "<w:pStyle w:val=\"ListParagraph\"/>"
               "<w:numPr>"
               "<w:ilvl w:val=\"" + std::to_string(m_level) + "\"/>"
               "<w:numId w:val=\"" + std::to_string(numId) + "\"/>"
               "</w:numPr>"
               "</w:pPr>"
               "<w:r><w:t xml:space=\"preserve\">" + xmlEscape(item) + "</w:t></w:r>"
               "</w:p>";
    }
    return xml;
}

// ---- Document ----

Document::Document() = default;
Document::~Document() = default;

Document& Document::setPage(const Page& page) {
    m_page = page;
    return *this;
}

Document& Document::setDefaultParagraphIndent(double chars, int fontSizePt) {
    if (chars <= 0) {
        m_defaultIndent = 0;
    } else {
        m_defaultIndent = static_cast<int>(chars * fontSizePt * 20);
    }
    return *this;
}

Document& Document::addHeading(const std::string& text, int level) {
    Element e;
    e.type = ElementType::Heading;
    e.text = text;
    e.headingLevel = level;
    m_elements.push_back(e);
    return *this;
}

Document& Document::addHeadingNoNum(const std::string& text, int level) {
    Element e;
    e.type = ElementType::Heading;
    e.text = text;
    e.headingLevel = level;
    e.noNumbering = true;
    m_elements.push_back(e);
    return *this;
}

Document& Document::setHeadingStyle(int level, const HeadingStyle& style) {
    if (level >= 1 && level <= 6) {
        m_headingStyles[level - 1] = style;
    }
    return *this;
}

Document& Document::enableHeadingNumbering() {
    m_headingNumbering = true;
    return *this;
}

Document& Document::disableHeadingNumbering() {
    m_headingNumbering = false;
    return *this;
}

Document& Document::enableImageNumbering(const std::string& prefix, CaptionNumStyle style) {
    m_imageNumbering = true;
    m_imageNumPrefix = prefix;
    m_imageNumStyle = style;
    return *this;
}

Document& Document::disableImageNumbering() {
    m_imageNumbering = false;
    return *this;
}

Document& Document::enableTableNumbering(const std::string& prefix, CaptionNumStyle style) {
    m_tableNumbering = true;
    m_tableNumPrefix = prefix;
    m_tableNumStyle = style;
    return *this;
}

Document& Document::disableTableNumbering() {
    m_tableNumbering = false;
    return *this;
}

Document& Document::setHeadingNumFormat(HeadingNumFormat fmt) {
    m_headingNumFormat = fmt;
    return *this;
}

Document& Document::addTOC(const std::string& levels, const std::string& title) {
    if (!title.empty()) {
        Element ht;
        ht.type = ElementType::Heading;
        ht.text = title;
        ht.headingLevel = 1;
        ht.noNumbering = true;  // TOC title should not be numbered
        m_elements.push_back(ht);
    }
    Element e;
    e.type = ElementType::TOC;
    e.extra = levels; // level range like "1-3"
    m_elements.push_back(e);
    return *this;
}

Paragraph& Document::addParagraph(const std::string& text) {
    auto p = std::make_unique<Paragraph>();
    if (!text.empty()) {
        p->addRun(text);
    }
    // Apply default indent
    if (m_defaultIndent > 0) {
        p->setFirstLineIndent(m_defaultIndent);
    }
    Paragraph* ptr = p.get();
    m_paragraphs.push_back(std::move(p));

    Element e;
    e.type = ElementType::Paragraph;
    e.data.paragraph = ptr;
    m_elements.push_back(e);
    return *ptr;
}

namespace {
// OPC part names must be valid URI segments (RFC 3986), so the in-zip filename
// must be pure ASCII with no reserved/space characters. Non-ASCII source
// filenames need to be remapped before being used as a part name.
bool isAsciiSafePartName(const std::string& s) {
    for (unsigned char c : s) {
        if (c < 0x21 || c >= 0x7F) return false;
        switch (c) {
            case '"': case '#': case '%': case '<': case '>':
            case '?': case '\\': case '`': case '{': case '}':
            case '|': case '^': case '[': case ']':
                return false;
        }
    }
    return !s.empty();
}
} // namespace

Image& Document::addImage(const std::string& filepath) {
    auto img = std::make_unique<Image>(filepath);
    namespace fs = std::filesystem;
    std::string filename = fs::u8path(filepath).filename().u8string();
    if (!isAsciiSafePartName(filename)) {
        std::string ext = fs::u8path(filepath).extension().u8string();
        if (!isAsciiSafePartName(ext)) ext.clear();
        img->setMediaName("image" + std::to_string(m_images.size() + 1) + ext);
    }
    Image* ptr = img.get();
    m_images.push_back(std::move(img));

    Element e;
    e.type = ElementType::Image;
    e.data.image = ptr;
    m_elements.push_back(e);
    return *ptr;
}

Image& Document::addImage(const std::filesystem::path& filepath) {
    return addImage(filepath.u8string());
}

Image& Document::addImage(const std::wstring& filepath) {
    return addImage(std::filesystem::path(filepath));
}

Table& Document::addTable(int rows, int cols) {
    auto tbl = std::make_unique<Table>(rows, cols);
    Table* ptr = tbl.get();
    m_tables.push_back(std::move(tbl));

    Element e;
    e.type = ElementType::Table;
    e.data.table = ptr;
    m_elements.push_back(e);
    return *ptr;
}

BulletList& Document::addBulletList() {
    auto lst = std::make_unique<BulletList>(ListType::Bullet);
    lst->setNumId(1); // all bullet lists share numId=1
    BulletList* ptr = lst.get();
    m_lists.push_back(std::move(lst));

    Element e;
    e.type = ElementType::BulletList;
    e.data.bulletList = ptr;
    m_elements.push_back(e);
    return *ptr;
}

BulletList& Document::addOrderedList() {
    auto lst = std::make_unique<BulletList>(ListType::Ordered);
    int id = m_nextOrderedListId++;
    lst->setNumId(id);
    BulletList* ptr = lst.get();
    m_lists.push_back(std::move(lst));

    Element e;
    e.type = ElementType::BulletList;
    e.data.bulletList = ptr;
    m_elements.push_back(e);
    return *ptr;
}

Equation& Document::addEquation(const std::string& latex) {
    auto eq = std::make_unique<Equation>(latex, EquationMode::Inline);
    Equation* ptr = eq.get();
    m_equations.push_back(std::move(eq));

    Element e;
    e.type = ElementType::Equation;
    e.data.equation = ptr;
    m_elements.push_back(e);
    return *ptr;
}

Equation& Document::addDisplayEquation(const std::string& latex) {
    auto eq = std::make_unique<Equation>(latex, EquationMode::Display);
    Equation* ptr = eq.get();
    m_equations.push_back(std::move(eq));

    Element e;
    e.type = ElementType::Equation;
    e.data.equation = ptr;
    m_elements.push_back(e);
    return *ptr;
}

void Document::buildDocumentXml(std::string& xml) {
    xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>";

    xml += "<w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\""
           " xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\""
           " xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\""
           " xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\""
           " xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\""
           " xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\">";

    xml += "<w:body>";

    int w = pageWidthDxa(m_page.size, m_page.orientation);
    int h = pageHeightDxa(m_page.size, m_page.orientation);
    auto mm = [](double cm) { return static_cast<int>(cm * 567.0); };

    // Content
    int chapter = 0;       // current H1 chapter (for ByChapter mode)
    int imgInChapter = 0;  // image counter within current chapter
    int tblInChapter = 0;  // table counter within current chapter
    int imgSeqNum = 0;     // global image counter (Sequential mode)
    int tblSeqNum = 0;     // global table counter (Sequential mode)

    auto formatCaptionNum = [&](CaptionNumStyle style, int chap, int chapIdx, int seqIdx) -> std::string {
        if (style == CaptionNumStyle::ByChapter) {
            int c = chap > 0 ? chap : 1;  // before any H1, fall back to chapter 1
            return std::to_string(c) + "-" + std::to_string(chapIdx);
        }
        return std::to_string(seqIdx);
    };

    auto buildCaption = [&](const std::string& prefix, const std::string& numText,
                            const std::string& userCaption) -> std::string {
        std::string s;
        s += "<w:p>"
             "<w:pPr><w:jc w:val=\"center\"/></w:pPr>"
             "<w:r><w:rPr><w:b/></w:rPr>"
             "<w:t xml:space=\"preserve\">" + xmlEscape(prefix + numText) + "</w:t>"
             "</w:r>";
        if (!userCaption.empty()) {
            s += "<w:r><w:rPr><w:b/></w:rPr>"
                 "<w:t xml:space=\"preserve\"> " + xmlEscape(userCaption) + "</w:t>"
                 "</w:r>";
        }
        s += "</w:p>";
        return s;
    };

    for (const auto& elem : m_elements) {
        switch (elem.type) {
            case ElementType::Heading: {
                // Track chapter on top-level (H1) headings, ignoring TOC title etc.
                if (elem.headingLevel == 1 && !elem.noNumbering) {
                    ++chapter;
                    imgInChapter = 0;
                    tblInChapter = 0;
                }
                std::string styleId = "Heading" + std::to_string(elem.headingLevel);
                // Check for custom alignment on this heading level
                const auto& hs = m_headingStyles[elem.headingLevel - 1];
                xml += "<w:p>"
                       "<w:pPr>"
                       "<w:pStyle w:val=\"" + styleId + "\"/>"
                       "<w:outlineLvl w:val=\"" + std::to_string(elem.headingLevel - 1) + "\"/>";
                if (hs.hasAlignment) {
                    xml += "<w:jc w:val=\"" + alignmentToString(hs.alignment) + "\"/>";
                }
                // Heading numbering: explicit numPr when enabled, numId=0 to suppress
                if (m_headingNumbering && !elem.noNumbering) {
                    xml += "<w:numPr>"
                           "<w:ilvl w:val=\"" + std::to_string(elem.headingLevel - 1) + "\"/>"
                           "<w:numId w:val=\"10\"/>"
                           "</w:numPr>";
                } else if (elem.noNumbering) {
                    xml += "<w:numPr><w:numId w:val=\"0\"/></w:numPr>";
                }
                xml += "</w:pPr>"
                       "<w:r><w:t xml:space=\"preserve\">" + xmlEscape(elem.text) + "</w:t></w:r>"
                       "</w:p>";
                break;
            }
            case ElementType::Paragraph: {
                xml += elem.data.paragraph->toXml();
                break;
            }
            case ElementType::Image: {
                Image* img = elem.data.image;
                // Find this image's index in m_images
                int idx = -1;
                for (size_t k = 0; k < m_images.size(); ++k) {
                    if (m_images[k].get() == img) { idx = static_cast<int>(k); break; }
                }
                int rIdNum = idx + 1;
                img->setRId("rId_img_" + std::to_string(rIdNum));

                // Auto-detect image size; constrain to page content width
                int marginTwips = static_cast<int>((m_page.margins.left + m_page.margins.right) * 567.0);
                int contentTwips = w - marginTwips;
                int maxWidthEmu = contentTwips * 635;  // 1 twip = 635 EMU
                auto imgSize = computeImageSize(img->filepath(), img->width(), img->height(), maxWidthEmu);
                std::string cx = std::to_string(imgSize.widthEmu);
                std::string cy = std::to_string(imgSize.heightEmu);

                xml += "<w:p>";
                if (img->hasAlignment()) {
                    xml += "<w:pPr><w:jc w:val=\"" + alignmentToString(img->alignment()) + "\"/></w:pPr>";
                }
                xml += "<w:r>"
                       "<w:drawing>"
                       "<wp:inline distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\">"
                       "<wp:extent cx=\"" + cx + "\" cy=\"" + cy + "\"/>"
                       "<wp:effectExtent l=\"0\" t=\"0\" r=\"0\" b=\"0\"/>"
                       "<wp:docPr id=\"" + std::to_string(rIdNum) + "\" name=\"Picture " + std::to_string(rIdNum) + "\"/>"
                       "<wp:cNvGraphicFramePr><a:graphicFrameLocks xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" noChangeAspect=\"1\"/></wp:cNvGraphicFramePr>"
                       "<a:graphic xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\">"
                       "<a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/picture\">"
                       "<pic:pic xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\">"
                       "<pic:nvPicPr>"
                       "<pic:cNvPr id=\"" + std::to_string(rIdNum) + "\" name=\"Picture " + std::to_string(rIdNum) + "\"/>"
                       "<pic:cNvPicPr/>"
                       "</pic:nvPicPr>"
                       "<pic:blipFill>"
                       "<a:blip r:embed=\"" + img->rId() + "\"/>"
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
                       "</w:r>"
                       "</w:p>";
                // Caption below image
                if (!img->caption().empty() || m_imageNumbering) {
                    if (m_imageNumbering) {
                        ++imgSeqNum;
                        ++imgInChapter;
                        std::string numText = formatCaptionNum(
                            m_imageNumStyle, chapter, imgInChapter, imgSeqNum);
                        xml += buildCaption(m_imageNumPrefix, numText, img->caption());
                    } else {
                        xml += buildCaption("", "", img->caption());
                    }
                }
                break;
            }
            case ElementType::Table: {
                Table* tbl = elem.data.table;
                // Table caption goes ABOVE the table
                if (!tbl->caption().empty() || m_tableNumbering) {
                    if (m_tableNumbering) {
                        ++tblSeqNum;
                        ++tblInChapter;
                        std::string numText = formatCaptionNum(
                            m_tableNumStyle, chapter, tblInChapter, tblSeqNum);
                        xml += buildCaption(m_tableNumPrefix, numText, tbl->caption());
                    } else {
                        xml += buildCaption("", "", tbl->caption());
                    }
                }
                xml += tbl->toXml();
                break;
            }
            case ElementType::BulletList: {
                xml += elem.data.bulletList->toXml();
                break;
            }
            case ElementType::Equation: {
                Equation* eq = elem.data.equation;
                if (eq->mode() == EquationMode::Display) {
                    xml += "<w:p>" + eq->toXml() + "</w:p>";
                } else {
                    xml += "<w:p><w:r>" + eq->toXml() + "</w:r></w:p>";
                }
                break;
            }
            case ElementType::TOC: {
                // TOC field: \o "1-3" \h \z \u
                xml += "<w:p>";
                xml += "<w:pPr><w:pStyle w:val=\"TOCHeading\"/></w:pPr>";
                xml += "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r>";
                xml += "<w:r><w:instrText xml:space=\"preserve\"> TOC \\o \"" + elem.extra + "\" \\h \\z \\u </w:instrText></w:r>";
                xml += "<w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>";
                xml += "<w:r><w:t xml:space=\"preserve\">[请在Word中右键点击此处更新目录]</w:t></w:r>";
                xml += "<w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
                xml += "</w:p>";
                break;
            }
        }
    }

    // Final section properties
    xml += "<w:sectPr>";
    xml += "<w:pgSz w:w=\"" + std::to_string(w)
         + "\" w:h=\"" + std::to_string(h) + "\"";
    if (m_page.orientation == Orientation::Landscape) {
        xml += " w:orient=\"landscape\"";
    }
    xml += "/>";
    xml += "<w:pgMar w:top=\"" + std::to_string(mm(m_page.margins.top))
         + "\" w:right=\"" + std::to_string(mm(m_page.margins.right))
         + "\" w:bottom=\"" + std::to_string(mm(m_page.margins.bottom))
         + "\" w:left=\"" + std::to_string(mm(m_page.margins.left))
         + "\" w:header=\"0\" w:footer=\"0\" w:gutter=\"0\"/>";
    xml += "</w:sectPr>";

    xml += "</w:body></w:document>";
}

std::string Document::buildRelationshipsXml() {
    std::string xml;
    xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
          "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">";

    // Image relationships (document-level)
    for (size_t i = 0; i < m_images.size(); ++i) {
        std::string rId = "rId_img_" + std::to_string(i + 1);
        xml += "<Relationship Id=\"" + rId + "\" "
               "Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/image\" "
               "Target=\"" + xmlEscape(m_images[i]->mediaPath()) + "\"/>";
    }

    // Image relationships (cell-level from tables)
    for (const auto& table : m_tables) {
        for (int r = 0; r < table->rows(); ++r) {
            for (int c = 0; c < table->cols(); ++c) {
                for (const auto& cimg : table->cell(r, c).images()) {
                    if (!cimg.rId.empty()) {
                        namespace fs = std::filesystem;
                        std::string mediaPath = "media/" + fs::u8path(cimg.filepath).filename().u8string();
                        xml += "<Relationship Id=\"" + cimg.rId + "\" "
                               "Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/image\" "
                               "Target=\"" + xmlEscape(mediaPath) + "\"/>";
                    }
                }
            }
        }
    }

    // Styles relationship
    xml += "<Relationship Id=\"rIdStyles\" "
           "Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles\" "
           "Target=\"styles.xml\"/>";

    // Numbering relationship
    xml += "<Relationship Id=\"rIdNum\" "
           "Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/numbering\" "
           "Target=\"numbering.xml\"/>";

    xml += "</Relationships>";
    return xml;
}

std::string Document::buildStylesXml() {
    // Default sizes per level
    const int defaultSizes[6] = {44, 36, 30, 28, 26, 24};
    const double defaultBefore[6] = {480, 360, 280, 240, 200, 200};
    const double defaultAfter[6] = {120, 80, 60, 40, 40, 40};

    auto makeHeadingStyle = [&](int level, int idx) {
        const auto& hs = m_headingStyles[idx];
        int sz = hs.fontSize > 0 ? hs.fontSize : defaultSizes[idx];
        int szHalf = sz * 2; // OOXML uses half-points

        double before = hs.spaceBefore >= 0 ? hs.spaceBefore : defaultBefore[idx];
        double after = hs.spaceAfter >= 0 ? hs.spaceAfter : defaultAfter[idx];

        std::string xml;
        xml += "<w:style w:type=\"paragraph\" w:styleId=\"Heading" + std::to_string(level) + "\">";
        xml += "<w:name w:val=\"heading " + std::to_string(level) + "\"/>";
        xml += "<w:basedOn w:val=\"Normal\"/>";
        xml += "<w:pPr>";
        xml += "<w:outlineLvl w:val=\"" + std::to_string(level - 1) + "\"/>";
        xml += "<w:ind w:firstLine=\"0\"/>"; // override Normal indent

        // Spacing (twips: 1pt = 20 twips)
        if (before > 0) xml += "<w:spacing w:before=\"" + std::to_string(static_cast<int>(before * 20)) + "\"/>";
        if (after > 0)  xml += "<w:spacing w:after=\"" + std::to_string(static_cast<int>(after * 20)) + "\"/>";

        // Line spacing (percentage: 240 = 1.0, 360 = 1.5, 480 = 2.0)
        if (hs.lineSpacing > 0) {
            int lineVal = static_cast<int>(hs.lineSpacing * 240);
            xml += "<w:spacing w:line=\"" + std::to_string(lineVal) + "\" w:lineRule=\"auto\"/>";
        }

        xml += "<w:keepNext/>";
        xml += "</w:pPr>";
        xml += "<w:rPr>";
        if (hs.bold) xml += "<w:b/>";
        if (hs.italic) xml += "<w:i/>";
        xml += "<w:sz w:val=\"" + std::to_string(szHalf) + "\"/>";
        xml += "<w:szCs w:val=\"" + std::to_string(szHalf) + "\"/>";
        if (!hs.color.empty()) xml += "<w:color w:val=\"" + hs.color + "\"/>";
        if (!hs.font.empty()) {
            xml += "<w:rFonts w:ascii=\"" + hs.font + "\" w:hAnsi=\"" + hs.font + "\" "
                   "w:eastAsia=\"" + hs.font + "\" w:cs=\"" + hs.font + "\"/>";
        }
        xml += "</w:rPr>";
        xml += "</w:style>";
        return xml;
    };

    std::string normalIndent;
    if (m_defaultIndent > 0) {
        normalIndent = "<w:ind w:firstLine=\"" + std::to_string(m_defaultIndent) + "\"/>";
    }

    std::string xml;
    xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
          "<w:styles xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\">"

          // Normal (default paragraph style)
          "<w:style w:type=\"paragraph\" w:styleId=\"Normal\" w:default=\"1\">"
          "<w:name w:val=\"Normal\"/>"
          "<w:pPr>"
          "<w:spacing w:after=\"160\" w:line=\"276\" w:lineRule=\"auto\"/>"
          + normalIndent +
          "</w:pPr>"
          "<w:rPr><w:sz w:val=\"22\"/><w:szCs w:val=\"22\"/></w:rPr>"
          "</w:style>"

          // List paragraph style
          "<w:style w:type=\"paragraph\" w:styleId=\"ListParagraph\">"
          "<w:name w:val=\"List Paragraph\"/>"
          "<w:basedOn w:val=\"Normal\"/>"
          "<w:pPr>"
          "<w:ind w:left=\"420\"/>"
          "</w:pPr>"
          "</w:style>"

          // TOC heading style
          "<w:style w:type=\"paragraph\" w:styleId=\"TOCHeading\">"
          "<w:name w:val=\"TOC Heading\"/>"
          "<w:basedOn w:val=\"Heading1\"/>"
          "<w:next w:val=\"Normal\"/>"
          "<w:pPr>"
          "<w:outlineLvl w:val=\"9\"/>"
          "<w:ind w:firstLine=\"0\"/>"
          "</w:pPr>"
          "</w:style>"

          // TOC level styles (used by Word when generating TOC entries)
          "<w:style w:type=\"paragraph\" w:styleId=\"TOC1\">"
          "<w:name w:val=\"TOC 1\"/>"
          "<w:basedOn w:val=\"Normal\"/>"
          "<w:pPr><w:ind w:firstLine=\"0\" w:left=\"0\"/></w:pPr>"
          "<w:rPr><w:b/></w:rPr>"
          "</w:style>"

          "<w:style w:type=\"paragraph\" w:styleId=\"TOC2\">"
          "<w:name w:val=\"TOC 2\"/>"
          "<w:basedOn w:val=\"Normal\"/>"
          "<w:pPr><w:ind w:firstLine=\"0\" w:left=\"240\"/></w:pPr>"
          "</w:style>"

          "<w:style w:type=\"paragraph\" w:styleId=\"TOC3\">"
          "<w:name w:val=\"TOC 3\"/>"
          "<w:basedOn w:val=\"Normal\"/>"
          "<w:pPr><w:ind w:firstLine=\"0\" w:left=\"480\"/></w:pPr>"
          "</w:style>"

          + makeHeadingStyle(1, 0) + makeHeadingStyle(2, 1)
          + makeHeadingStyle(3, 2) + makeHeadingStyle(4, 3)
          + makeHeadingStyle(5, 4) + makeHeadingStyle(6, 5)

          + "</w:styles>";
    return xml;
}

std::string Document::buildHeadingNumberingXml() {
    std::string xml;
    xml += "<w:abstractNum w:abstractNumId=\"10\">"
           "<w:multiLevelType w:val=\"hybridMultilevel\"/>";

    // Level 1 text depends on format
    std::string l1Text = (m_headingNumFormat == HeadingNumFormat::Chapter)
        ? "\xE7\xAC\xAC%1\xE7\xAB\xA0"  // 第%1章
        : "%1";

    const char* lvlTexts[] = {nullptr, "%1.%2", "%1.%2.%3",
                              "%1.%2.%3.%4", "%1.%2.%3.%4.%5",
                              "%1.%2.%3.%4.%5.%6"};
    for (int i = 0; i < 6; ++i) {
        std::string lvlText = (i == 0) ? l1Text : lvlTexts[i];
        xml += "<w:lvl w:ilvl=\"" + std::to_string(i) + "\" w:tplc=\"0\">"
               "<w:start w:val=\"1\"/>"
               "<w:numFmt w:val=\"decimal\"/>"
               "<w:lvlText w:val=\"" + lvlText + "\"/>"
               "<w:lvlJc w:val=\"left\"/>"
               "<w:pPr>"
               "<w:ind w:left=\"" + std::to_string((i + 1) * 360) + "\" w:hanging=\"360\"/>"
               "</w:pPr>"
               "</w:lvl>";
    }
    xml += "</w:abstractNum>";
    return xml;
}

std::string Document::buildNumberingXml() {
    std::string xml;
    xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
          "<w:numbering xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\">";

    // --- Abstract 0: Bullet list (· 不编号，按列表格式) ---
    // Define all 9 levels so Word does not fall back to Heading numbering
    xml += "<w:abstractNum w:abstractNumId=\"0\">"
           "<w:multiLevelType w:val=\"hybridMultilevel\"/>";
    for (int i = 0; i < 9; ++i) {
        xml += "<w:lvl w:ilvl=\"" + std::to_string(i) + "\" w:tplc=\"0\">"
               "<w:start w:val=\"1\"/>"
               "<w:numFmt w:val=\"bullet\"/>"
               "<w:lvlText w:val=\"\xC2\xB7\"/>"   // · (U+00B7) middle dot
               "<w:lvlJc w:val=\"left\"/>"
               "<w:pPr>"
               "<w:ind w:left=\"" + std::to_string((i + 1) * 420) + "\" w:hanging=\"420\"/>"
               "</w:pPr>"
               "<w:rPr><w:rFonts w:hint=\"default\"/></w:rPr>"
               "</w:lvl>";
    }
    xml += "</w:abstractNum>";

    // --- Abstract 1: Ordered list (1. 2. 3. ...) ---
    xml += "<w:abstractNum w:abstractNumId=\"1\">"
           "<w:multiLevelType w:val=\"hybridMultilevel\"/>";
    for (int i = 0; i < 9; ++i) {
        xml += "<w:lvl w:ilvl=\"" + std::to_string(i) + "\" w:tplc=\"0\">"
               "<w:start w:val=\"1\"/>"
               "<w:numFmt w:val=\"decimal\"/>"
               "<w:lvlText w:val=\"%" + std::to_string(i + 1) + ".\"/>"
               "<w:lvlJc w:val=\"left\"/>"
               "<w:pPr>"
               "<w:ind w:left=\"" + std::to_string((i + 1) * 420) + "\" w:hanging=\"420\"/>"
               "</w:pPr>"
               "</w:lvl>";
    }
    xml += "</w:abstractNum>";

    // --- Heading abstractNum (only when needed) ---
    if (m_headingNumbering) {
        xml += buildHeadingNumberingXml();
    }

    // --- Num instances (must come AFTER all abstractNum definitions) ---
    // numId=1: Bullet list (shared by all bullet lists)
    xml += "<w:num w:numId=\"1\">"
           "<w:abstractNumId w:val=\"0\"/>"
           "</w:num>";

    // numId=3+: Each ordered list gets its own restart-from-1
    for (int id = 3; id < m_nextOrderedListId; ++id) {
        xml += "<w:num w:numId=\"" + std::to_string(id) + "\">"
               "<w:abstractNumId w:val=\"1\"/>"
               "<w:lvlOverride w:ilvl=\"0\">"
               "<w:startOverride w:val=\"1\"/>"
               "</w:lvlOverride>"
               "</w:num>";
    }

    // numId=10: Heading numbering instance
    if (m_headingNumbering) {
        xml += "<w:num w:numId=\"10\">"
               "<w:abstractNumId w:val=\"10\"/>"
               "</w:num>";
    }

    xml += "</w:numbering>";
    return xml;
}

std::string Document::buildContentTypesXml() {
    std::string xml;
    xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
          "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
          "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>"
          "<Default Extension=\"xml\" ContentType=\"application/xml\"/>"
          "<Override PartName=\"/word/document.xml\" "
          "ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml\"/>"
          "<Override PartName=\"/word/numbering.xml\" "
          "ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.numbering+xml\"/>"
          "<Override PartName=\"/word/styles.xml\" "
          "ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml\"/>";

    // Image types (document-level)
    for (const auto& img : m_images) {
        namespace fs = std::filesystem;
        std::string ext = fs::u8path(img->filepath()).extension().u8string();
        if (!ext.empty()) ext = ext.substr(1);
        xml += "<Override PartName=\"/word/" + img->mediaPath() + "\" "
               "ContentType=\"" + imageContentType(ext) + "\"/>";
    }

    // Image types (cell-level from tables)
    for (const auto& table : m_tables) {
        for (int r = 0; r < table->rows(); ++r) {
            for (int c = 0; c < table->cols(); ++c) {
                for (const auto& cimg : table->cell(r, c).images()) {
                    if (!cimg.rId.empty()) {
                        namespace fs = std::filesystem;
                        std::string mediaPath = "media/" + fs::u8path(cimg.filepath).filename().u8string();
                        std::string ext = fs::u8path(cimg.filepath).extension().u8string();
                        if (!ext.empty()) ext = ext.substr(1);
                        xml += "<Override PartName=\"/word/" + mediaPath + "\" "
                               "ContentType=\"" + imageContentType(ext) + "\"/>";
                    }
                }
            }
        }
    }

    xml += "</Types>";
    return xml;
}

bool Document::save(const std::string& filepath) {
    try {
        // Collect and register cell images from all tables
        std::vector<CellImage*> cellImages;
        for (auto& table : m_tables) {
            table->collectCellImages(cellImages);
        }

        // Assign rIds to cell images
        int imgIndex = static_cast<int>(m_images.size());
        for (size_t i = 0; i < cellImages.size(); ++i) {
            cellImages[i]->rId = "rId_img_" + std::to_string(imgIndex + static_cast<int>(i) + 1);
        }

        ZipWriter zip(filepath);

        // Build document.xml
        std::string docXml;
        buildDocumentXml(docXml);

        // Relationships
        std::string relsXml = buildRelationshipsXml();

        // Content Types
        std::string contentTypesXml = buildContentTypesXml();

        // .rels
        std::string dotRels = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
            "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
            "<Relationship Id=\"rId1\" "
            "Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" "
            "Target=\"word/document.xml\"/>"
            "</Relationships>";

        // Add entries to ZIP
        zip.addEntry("[Content_Types].xml", contentTypesXml);
        zip.addEntry("_rels/.rels", dotRels);
        zip.addEntry("word/document.xml", docXml);
        zip.addEntry("word/_rels/document.xml.rels", relsXml);
        zip.addEntry("word/styles.xml", buildStylesXml());
        zip.addEntry("word/numbering.xml", buildNumberingXml());

        // Add document-level images
        for (const auto& img : m_images) {
            if (!zip.addFileEntry("word/" + img->mediaPath(), img->filepath())) {
                return false;
            }
        }

        // Add cell images from tables
        for (auto* cimg : cellImages) {
            namespace fs = std::filesystem;
            std::string mediaPath = "media/" + fs::u8path(cimg->filepath).filename().u8string();
            if (!zip.addFileEntry("word/" + mediaPath, cimg->filepath)) {
                return false;
            }
        }

        zip.finalize();
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

} // namespace xword
