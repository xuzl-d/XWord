#include "xword/Document.hpp"
#include "internal/ZipWriter.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <filesystem>

namespace xword {

using namespace internal;

enum class ElementType { Heading, Paragraph, Image, Table, BulletList, Equation, TOC, SectionBreak };

struct Element {
    ElementType type; int headingLevel=0; std::string text,extra; bool noNumbering=false;
    union Data { Paragraph* paragraph=nullptr; class Image* image; class Table* table;
        class BulletList* bulletList; class Equation* equation; void* ptr; Data(){} } data;
};

struct FootnoteInfo {
    int id;
    std::string text;
};

struct SectionInfo {
    Page             page;
    std::unique_ptr<Paragraph> header;
    std::unique_ptr<Paragraph> footer;
    SectionBreakType breakType = SectionBreakType::NextPage;
    bool             titlePg = false;
};

struct Document::Impl {
    std::vector<std::unique_ptr<Paragraph>>   m_paragraphs;
    std::vector<std::unique_ptr<Image>>       m_images;
    std::vector<std::unique_ptr<Table>>       m_tables;
    std::vector<std::unique_ptr<BulletList>>  m_lists;
    std::vector<std::unique_ptr<Equation>>    m_equations;
    std::vector<Element>                      m_elements;
    HeadingStyle    m_headingStyles[6];
    bool            m_headingNumbering = false;
    HeadingNumFormat m_headingNumFormat = HeadingNumFormat::Decimal;
    int             m_nextOrderedListId = 3;
    int             m_defaultIndent = 480;
    bool            m_imageNumbering = false;
    std::string     m_imageNumPrefix = "å¾";
    CaptionNumStyle m_imageNumStyle = CaptionNumStyle::Sequential;
    bool            m_tableNumbering = false;
    std::string     m_tableNumPrefix = "è¡¨";
    CaptionNumStyle m_tableNumStyle = CaptionNumStyle::Sequential;
    std::vector<FootnoteInfo>   m_footnotes;
    int                         m_nextFootnoteId = 1;
    std::vector<SectionInfo>    m_sections;
    int                         m_curSection = 0; // index into m_sections
    bool            m_isTemplate = false;
    std::unordered_map<std::string, std::string> m_templateParts, m_templateVars;
};

// ---- Document ----

Document::Document() : m_impl(std::make_unique<Impl>()) {
    m_impl->m_sections.emplace_back(); // default section 0
}
Document::~Document() = default;

Document& Document::setPage(const Page& page) {
    m_impl->m_sections[m_impl->m_curSection].page = page;
    return *this;
}

Document& Document::setDefaultParagraphIndent(double chars, int fontSizePt) {
    if (chars <= 0) {
        m_impl->m_defaultIndent = 0;
    } else {
        m_impl->m_defaultIndent = static_cast<int>(chars * fontSizePt * 20);
    }
    return *this;
}

Document& Document::addHeading(const std::string& text, int level) {
    Element e;
    e.type = ElementType::Heading;
    e.text = text;
    e.headingLevel = level;
    m_impl->m_elements.push_back(e);
    return *this;
}

Document& Document::addHeadingNoNum(const std::string& text, int level) {
    Element e;
    e.type = ElementType::Heading;
    e.text = text;
    e.headingLevel = level;
    e.noNumbering = true;
    m_impl->m_elements.push_back(e);
    return *this;
}

Document& Document::setHeadingStyle(int level, const HeadingStyle& style) {
    if (level >= 1 && level <= 6) {
        m_impl->m_headingStyles[level - 1] = style;
    }
    return *this;
}

Document& Document::enableHeadingNumbering() {
    m_impl->m_headingNumbering = true;
    return *this;
}

Document& Document::disableHeadingNumbering() {
    m_impl->m_headingNumbering = false;
    return *this;
}

Document& Document::enableImageNumbering(const std::string& prefix, CaptionNumStyle style) {
    m_impl->m_imageNumbering = true;
    m_impl->m_imageNumPrefix = prefix;
    m_impl->m_imageNumStyle = style;
    return *this;
}

Document& Document::disableImageNumbering() {
    m_impl->m_imageNumbering = false;
    return *this;
}

Document& Document::enableTableNumbering(const std::string& prefix, CaptionNumStyle style) {
    m_impl->m_tableNumbering = true;
    m_impl->m_tableNumPrefix = prefix;
    m_impl->m_tableNumStyle = style;
    return *this;
}

Document& Document::disableTableNumbering() {
    m_impl->m_tableNumbering = false;
    return *this;
}

Document& Document::setHeadingNumFormat(HeadingNumFormat fmt) {
    m_impl->m_headingNumFormat = fmt;
    return *this;
}

Document& Document::setHeader(const std::string& text) {
    auto& sec = m_impl->m_sections[m_impl->m_curSection];
    sec.header = std::make_unique<Paragraph>();
    if (!text.empty()) sec.header->addRun(text);
    sec.header->setAlignment(Alignment::Center);
    sec.header->setFirstLineIndent(0);
    return *this;
}

Document& Document::setFooter(const std::string& text) {
    auto& sec = m_impl->m_sections[m_impl->m_curSection];
    sec.footer = std::make_unique<Paragraph>();
    if (!text.empty()) sec.footer->addRun(text);
    sec.footer->setAlignment(Alignment::Center);
    sec.footer->setFirstLineIndent(0);
    return *this;
}

Paragraph& Document::setHeader() {
    auto& sec = m_impl->m_sections[m_impl->m_curSection];
    sec.header = std::make_unique<Paragraph>();
    sec.header->setFirstLineIndent(0);
    return *sec.header;
}

Paragraph& Document::setFooter() {
    auto& sec = m_impl->m_sections[m_impl->m_curSection];
    sec.footer = std::make_unique<Paragraph>();
    sec.footer->setFirstLineIndent(0);
    return *sec.footer;
}

void Document::clearHeader() {
    m_impl->m_sections[m_impl->m_curSection].header.reset();
}

void Document::clearFooter() {
    m_impl->m_sections[m_impl->m_curSection].footer.reset();
}

Document& Document::addTOC(const std::string& levels, const std::string& title) {
    if (!title.empty()) {
        Element ht;
        ht.type = ElementType::Heading;
        ht.text = title;
        ht.headingLevel = 1;
        ht.noNumbering = true;  // TOC title should not be numbered
        m_impl->m_elements.push_back(ht);
    }
    Element e;
    e.type = ElementType::TOC;
    e.extra = levels; // level range like "1-3"
    m_impl->m_elements.push_back(e);
    return *this;
}

Paragraph& Document::addParagraph(const std::string& text) {
    auto p = std::make_unique<Paragraph>();
    if (!text.empty()) {
        p->addRun(text);
    }
    // Apply default indent
    if (m_impl->m_defaultIndent > 0) {
        p->setFirstLineIndent(m_impl->m_defaultIndent);
    }
    Paragraph* ptr = p.get();
    m_impl->m_paragraphs.push_back(std::move(p));

    Element e;
    e.type = ElementType::Paragraph;
    e.data.paragraph = ptr;
    m_impl->m_elements.push_back(e);
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
        img->setMediaName("image" + std::to_string(m_impl->m_images.size() + 1) + ext);
    }
    Image* ptr = img.get();
    m_impl->m_images.push_back(std::move(img));

    Element e;
    e.type = ElementType::Image;
    e.data.image = ptr;
    m_impl->m_elements.push_back(e);
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
    m_impl->m_tables.push_back(std::move(tbl));

    Element e;
    e.type = ElementType::Table;
    e.data.table = ptr;
    m_impl->m_elements.push_back(e);
    return *ptr;
}

BulletList& Document::addBulletList() {
    auto lst = std::make_unique<BulletList>(ListType::Bullet);
    lst->setNumId(1); // all bullet lists share numId=1
    BulletList* ptr = lst.get();
    m_impl->m_lists.push_back(std::move(lst));

    Element e;
    e.type = ElementType::BulletList;
    e.data.bulletList = ptr;
    m_impl->m_elements.push_back(e);
    return *ptr;
}

BulletList& Document::addOrderedList() {
    auto lst = std::make_unique<BulletList>(ListType::Ordered);
    int id = m_impl->m_nextOrderedListId++;
    lst->setNumId(id);
    BulletList* ptr = lst.get();
    m_impl->m_lists.push_back(std::move(lst));

    Element e;
    e.type = ElementType::BulletList;
    e.data.bulletList = ptr;
    m_impl->m_elements.push_back(e);
    return *ptr;
}

Equation& Document::addEquation(const std::string& latex) {
    auto eq = std::make_unique<Equation>(latex, EquationMode::Inline);
    Equation* ptr = eq.get();
    m_impl->m_equations.push_back(std::move(eq));

    Element e;
    e.type = ElementType::Equation;
    e.data.equation = ptr;
    m_impl->m_elements.push_back(e);
    return *ptr;
}

Equation& Document::addDisplayEquation(const std::string& latex) {
    auto eq = std::make_unique<Equation>(latex, EquationMode::Display);
    Equation* ptr = eq.get();
    m_impl->m_equations.push_back(std::move(eq));

    Element e;
    e.type = ElementType::Equation;
    e.data.equation = ptr;
    m_impl->m_elements.push_back(e);
    return *ptr;
}

Document& Document::addSectionBreak(SectionBreakType type) {
    // Add section-break marker element
    Element e;
    e.type = ElementType::SectionBreak;
    m_impl->m_elements.push_back(e);

    // Create new section inheriting current section's page settings
    int cur = m_impl->m_curSection;
    SectionInfo newSec;
    newSec.page     = m_impl->m_sections[cur].page;
    newSec.breakType = type;
    m_impl->m_sections.push_back(std::move(newSec));
    m_impl->m_curSection = static_cast<int>(m_impl->m_sections.size()) - 1;
    return *this;
}

Document& Document::enableTitlePage() {
    m_impl->m_sections[m_impl->m_curSection].titlePg = true;
    return *this;
}

int Document::addFootnote(const std::string& text) {
    int id = m_impl->m_nextFootnoteId++;
    m_impl->m_footnotes.push_back({id, text});
    return id;
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

    auto mm = [](double cm) { return static_cast<int>(cm * 567.0); };

    // Content
    int chapter = 0;       // current H1 chapter (for ByChapter mode)
    int imgInChapter = 0;  // image counter within current chapter
    int tblInChapter = 0;  // table counter within current chapter
    int imgSeqNum = 0;     // global image counter (Sequential mode)
    int tblSeqNum = 0;     // global table counter (Sequential mode)

    auto formatCaptionNum = [&](CaptionNumStyle style, int chap, int chapIdx, int seqIdx) -> std::string {
        if (style == CaptionNumStyle::ByChapter) {
            int c = chap > 0 ? chap : 1;
            return std::to_string(c) + "-" + std::to_string(chapIdx);
        }
        return std::to_string(seqIdx);
    };

    auto buildCaption = [&](const std::string& prefix, const std::string& numText,
                            const std::string& userCaption) -> std::string {
        std::string s;
        s += "<w:p>"
             "<w:pPr><w:jc w:val=\"center\"/><w:ind w:firstLine=\"0\"/></w:pPr>"
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

    // Build sectPr XML for a given section.
    // firstSection: suppress w:type for the document's opening section.
    auto buildSectPr = [&](const SectionInfo& si, const std::string& hdrRId,
                           const std::string& ftrRId, bool firstSection) -> std::string {
        int w = pageWidthDxa(si.page.size, si.page.orientation);
        int h = pageHeightDxa(si.page.size, si.page.orientation);

        std::string s;

        // 1. headerReference(s) — default only for now
        if (!hdrRId.empty())
            s += "<w:headerReference w:type=\"default\" r:id=\"" + hdrRId + "\"/>";

        // 2. footerReference(s) — default only for now
        if (!ftrRId.empty())
            s += "<w:footerReference w:type=\"default\" r:id=\"" + ftrRId + "\"/>";

        // 3. type (section-break type) — only for non-first sections
        if (!firstSection)
            s += "<w:type w:val=\"" + sectionBreakTypeToString(si.breakType) + "\"/>";

        // 4. pgSz (page size)
        s += "<w:pgSz w:w=\"" + std::to_string(w) + "\""
           + " w:h=\"" + std::to_string(h) + "\"";
        if (si.page.orientation == Orientation::Landscape)
            s += " w:orient=\"landscape\"";
        s += "/>";

        // 5. pgMar (page margins)
        s += "<w:pgMar w:top=\"" + std::to_string(mm(si.page.margins.top))
           + "\" w:right=\"" + std::to_string(mm(si.page.margins.right))
           + "\" w:bottom=\"" + std::to_string(mm(si.page.margins.bottom))
           + "\" w:left=\"" + std::to_string(mm(si.page.margins.left))
           + "\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/>";

        // 6. cols (column definition)
        s += "<w:cols w:space=\"720\"/>";

        // 7. titlePg (different first page)
        if (si.titlePg)
            s += "<w:titlePg/>";

        // 8. docGrid (document grid)
        s += "<w:docGrid w:linePitch=\"360\"/>";

        return s;
    };

    // Partition elements into sections
    struct SecGroup { const SectionInfo* info = nullptr; std::vector<const Element*> elems; };
    std::vector<SecGroup> secGroups;
    secGroups.push_back({&m_impl->m_sections[0], {}});

    for (const auto& elem : m_impl->m_elements) {
        if (elem.type == ElementType::SectionBreak) {
            int nextIdx = static_cast<int>(secGroups.size());
            secGroups.push_back({&m_impl->m_sections[nextIdx], {}});
        } else {
            secGroups.back().elems.push_back(&elem);
        }
    }

    for (size_t si = 0; si < secGroups.size(); ++si) {
        const auto& group = secGroups[si];
        const SectionInfo& sec = *group.info;
        int secW = pageWidthDxa(sec.page.size, sec.page.orientation);

        // Emit elements for this section
        size_t secXmlStart = xml.size();

        for (const auto* elem : group.elems) {
            switch (elem->type) {
                case ElementType::Heading: {
                    if (elem->headingLevel == 1 && !elem->noNumbering) {
                        ++chapter;
                        imgInChapter = 0;
                        tblInChapter = 0;
                    }
                    std::string styleId = "Heading" + std::to_string(elem->headingLevel);
                    const auto& hs = m_impl->m_headingStyles[elem->headingLevel - 1];
                    xml += "<w:p>"
                           "<w:pPr>"
                           "<w:pStyle w:val=\"" + styleId + "\"/>"
                           "<w:outlineLvl w:val=\"" + std::to_string(elem->headingLevel - 1) + "\"/>";
                    if (hs.hasAlignment)
                        xml += "<w:jc w:val=\"" + alignmentToString(hs.alignment) + "\"/>";
                    if (m_impl->m_headingNumbering && !elem->noNumbering) {
                        xml += "<w:numPr>"
                               "<w:ilvl w:val=\"" + std::to_string(elem->headingLevel - 1) + "\"/>"
                               "<w:numId w:val=\"10\"/>"
                               "</w:numPr>";
                    } else if (elem->noNumbering) {
                        xml += "<w:numPr><w:numId w:val=\"0\"/></w:numPr>";
                    }
                    xml += "</w:pPr>"
                           "<w:r><w:t xml:space=\"preserve\">" + xmlEscape(elem->text) + "</w:t></w:r>"
                           "</w:p>";
                    break;
                }
                case ElementType::Paragraph: {
                    xml += elem->data.paragraph->toXml();
                    break;
                }
                case ElementType::Image: {
                    Image* img = elem->data.image;
                    if (img->skipped()) break;
                    int idx = -1;
                    for (size_t k = 0; k < m_impl->m_images.size(); ++k) {
                        if (m_impl->m_images[k].get() == img) { idx = static_cast<int>(k); break; }
                    }
                    int rIdNum = idx + 1;
                    img->setRId("rId_img_" + std::to_string(rIdNum));

                    int marginTwips = static_cast<int>((sec.page.margins.left + sec.page.margins.right) * 567.0);
                    int contentTwips = secW - marginTwips;
                    int maxWidthEmu = contentTwips * 635;
                    auto imgSize = computeImageSize(img->filepath(), img->width(), img->height(), maxWidthEmu);
                    std::string cx = std::to_string(imgSize.widthEmu);
                    std::string cy = std::to_string(imgSize.heightEmu);

                    xml += "<w:p>";
                    xml += "<w:pPr>";
                    if (img->hasAlignment())
                        xml += "<w:jc w:val=\"" + alignmentToString(img->alignment()) + "\"/>";
                    xml += "<w:ind w:firstLine=\"0\"/>";
                    xml += "</w:pPr>";
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
                    if (!img->caption().empty() || m_impl->m_imageNumbering) {
                        if (m_impl->m_imageNumbering) {
                            ++imgSeqNum;
                            ++imgInChapter;
                            std::string numText = formatCaptionNum(
                                m_impl->m_imageNumStyle, chapter, imgInChapter, imgSeqNum);
                            xml += buildCaption(m_impl->m_imageNumPrefix, numText, img->caption());
                        } else {
                            xml += buildCaption("", "", img->caption());
                        }
                    }
                    break;
                }
                case ElementType::Table: {
                    Table* tbl = elem->data.table;
                    if (!tbl->caption().empty() || m_impl->m_tableNumbering) {
                        if (m_impl->m_tableNumbering) {
                            ++tblSeqNum;
                            ++tblInChapter;
                            std::string numText = formatCaptionNum(
                                m_impl->m_tableNumStyle, chapter, tblInChapter, tblSeqNum);
                            xml += buildCaption(m_impl->m_tableNumPrefix, numText, tbl->caption());
                        } else {
                            xml += buildCaption("", "", tbl->caption());
                        }
                    }
                    xml += tbl->toXml();
                    break;
                }
                case ElementType::BulletList: {
                    xml += elem->data.bulletList->toXml();
                    break;
                }
                case ElementType::Equation: {
                    Equation* eq = elem->data.equation;
                    if (eq->mode() == EquationMode::Display)
                        xml += "<w:p>" + eq->toXml() + "</w:p>";
                    else
                        xml += "<w:p><w:r>" + eq->toXml() + "</w:r></w:p>";
                    break;
                }
                case ElementType::TOC: {
                    xml += "<w:p>";
                    xml += "<w:pPr><w:pStyle w:val=\"TOCHeading\"/></w:pPr>";
                    xml += "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r>";
                    xml += "<w:r><w:instrText xml:space=\"preserve\"> TOC \\o \"" + elem->extra + "\" \\h \\z \\u </w:instrText></w:r>";
                    xml += "<w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>";
                    xml += "<w:r><w:t xml:space=\"preserve\">[请在Word中右键点击此处更新目录]</w:t></w:r>";
                    xml += "<w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
                    xml += "</w:p>";
                    break;
                }
                case ElementType::SectionBreak:
                    break; // handled by section partitioning
            }
        }

        // Determine header/footer rIds for this section
        std::string hdrRId, ftrRId;
        if (sec.header) {
            hdrRId = "rIdHeader";
            if (si > 0) hdrRId += std::to_string(si + 1);
        }
        if (sec.footer) {
            ftrRId = "rIdFooter";
            if (si > 0) ftrRId += std::to_string(si + 1);
        }

        std::string sectPrXml = buildSectPr(sec, hdrRId, ftrRId, si == 0);

        if (si < secGroups.size() - 1) {
            // Non-final section: sectPr goes inside last paragraph's pPr.
            size_t lastPEnd = xml.rfind("</w:p>");

            if (lastPEnd != std::string::npos && lastPEnd >= secXmlStart) {
                // A paragraph exists in this section — inject sectPr into it.
                size_t lastPStart = xml.rfind("<w:p", lastPEnd);
                size_t pPrClose = xml.rfind("</w:pPr>", lastPEnd);

                if (pPrClose != std::string::npos && pPrClose > lastPStart) {
                    // Has pPr: insert sectPr before </w:pPr>
                    xml.insert(pPrClose, "<w:sectPr>" + sectPrXml + "</w:sectPr>");
                } else {
                    // No pPr: add one before the first <w:r> or before </w:p>
                    size_t firstR = xml.find("<w:r", lastPStart);
                    if (firstR != std::string::npos && firstR < lastPEnd) {
                        xml.insert(firstR,
                            "<w:pPr><w:sectPr>" + sectPrXml + "</w:sectPr></w:pPr>");
                    } else {
                        xml.insert(lastPEnd,
                            "<w:pPr><w:sectPr>" + sectPrXml + "</w:sectPr></w:pPr>");
                    }
                }
            } else {
                // No paragraph in this section — emit an empty one to carry
                // the section properties (Word requires a paragraph for sectPr).
                xml += "<w:p><w:pPr><w:sectPr>" + sectPrXml
                     + "</w:sectPr></w:pPr></w:p>";
            }
        } else {
            // Final section: sectPr at body level
            xml += "<w:sectPr>" + sectPrXml + "</w:sectPr>";
        }
    }

    xml += "</w:body></w:document>";
}

std::string Document::buildRelationshipsXml() {
    std::string xml;
    xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
          "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">";

    // Image relationships (document-level)
    for (size_t i = 0; i < m_impl->m_images.size(); ++i) {
        if (m_impl->m_images[i]->skipped()) continue;
        std::string rId = "rId_img_" + std::to_string(i + 1);
        xml += "<Relationship Id=\"" + rId + "\" "
               "Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/image\" "
               "Target=\"" + xmlEscape(m_impl->m_images[i]->mediaPath()) + "\"/>";
    }

    // Image relationships (cell-level from tables)
    for (const auto& table : m_impl->m_tables) {
        for (int r = 0; r < table->rows(); ++r) {
            for (int c = 0; c < table->cols(); ++c) {
                for (const auto& cimg : table->cell(r, c).images()) {
                    if (cimg.skipped) continue;
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

    // Header / footer relationships (per-section)
    for (size_t si = 0; si < m_impl->m_sections.size(); ++si) {
        const auto& sec = m_impl->m_sections[si];
        std::string hdrRId = "rIdHeader";
        std::string ftrRId = "rIdFooter";
        if (si > 0) {
            hdrRId += std::to_string(si + 1);
            ftrRId += std::to_string(si + 1);
        }
        if (sec.header) {
            std::string target = "header" + std::to_string(si + 1) + ".xml";
            xml += "<Relationship Id=\"" + hdrRId + "\" "
                   "Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/header\" "
                   "Target=\"" + target + "\"/>";
        }
        if (sec.footer) {
            std::string target = "footer" + std::to_string(si + 1) + ".xml";
            xml += "<Relationship Id=\"" + ftrRId + "\" "
                   "Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/footer\" "
                   "Target=\"" + target + "\"/>";
        }
    }

    // Footnotes relationship
    if (!m_impl->m_footnotes.empty()) {
        xml += "<Relationship Id=\"rIdFootnotes\" "
               "Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/footnotes\" "
               "Target=\"footnotes.xml\"/>";
    }

    xml += "</Relationships>";
    return xml;
}

std::string Document::buildStylesXml() {
    // Default sizes per level
    const int defaultSizes[6] = {22, 16, 14, 13, 12, 11};
    const double defaultBefore[6] = {18, 14, 12, 10, 8, 8};
    const double defaultAfter[6] = {6, 5, 4, 3, 3, 3};

    auto makeHeadingStyle = [&](int level, int idx) {
        const auto& hs = m_impl->m_headingStyles[idx];
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
    if (m_impl->m_defaultIndent > 0) {
        normalIndent = "<w:ind w:firstLine=\"" + std::to_string(m_impl->m_defaultIndent) + "\"/>";
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

          // Footnote Reference (superscript style in body text and footnote)
          "<w:style w:type=\"character\" w:styleId=\"FootnoteReference\">"
          "<w:name w:val=\"footnote reference\"/>"
          "<w:rPr>"
          "<w:vertAlign w:val=\"superscript\"/>"
          "<w:sz w:val=\"18\"/><w:szCs w:val=\"18\"/>"
          "</w:rPr>"
          "</w:style>"

          // Footnote Text paragraph style
          "<w:style w:type=\"paragraph\" w:styleId=\"FootnoteText\">"
          "<w:name w:val=\"footnote text\"/>"
          "<w:basedOn w:val=\"Normal\"/>"
          "<w:pPr>"
          "<w:spacing w:after=\"0\" w:line=\"240\" w:lineRule=\"auto\"/>"
          "<w:ind w:firstLine=\"0\"/>"
          "</w:pPr>"
          "<w:rPr><w:sz w:val=\"18\"/><w:szCs w:val=\"18\"/></w:rPr>"
          "</w:style>"

          + makeHeadingStyle(1, 0) + makeHeadingStyle(2, 1)
          + makeHeadingStyle(3, 2) + makeHeadingStyle(4, 3)
          + makeHeadingStyle(5, 4) + makeHeadingStyle(6, 5)

          + "</w:styles>";
    return xml;
}

std::string Document::buildFootnotesXml() {
    if (m_impl->m_footnotes.empty()) return {};

    std::string xml;
    xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
          "<w:footnotes xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\""
          " xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">";

    // Separator footnote (id=-1, type=separator)
    xml += "<w:footnote w:type=\"separator\" w:id=\"-1\">"
           "<w:p>"
           "<w:pPr><w:spacing w:after=\"0\" w:line=\"240\" w:lineRule=\"auto\"/></w:pPr>"
           "<w:r><w:separator/></w:r>"
           "</w:p>"
           "</w:footnote>";

    // Continuation separator footnote (id=0, type=continuationSeparator)
    xml += "<w:footnote w:type=\"continuationSeparator\" w:id=\"0\">"
           "<w:p>"
           "<w:pPr><w:spacing w:after=\"0\" w:line=\"240\" w:lineRule=\"auto\"/></w:pPr>"
           "<w:r><w:continuationSeparator/></w:r>"
           "</w:p>"
           "</w:footnote>";

    // Normal footnotes
    for (const auto& fn : m_impl->m_footnotes) {
        xml += "<w:footnote w:id=\"" + std::to_string(fn.id) + "\">"
               "<w:p>"
               "<w:pPr><w:pStyle w:val=\"FootnoteText\"/></w:pPr>"
               "<w:r>"
               "<w:rPr><w:rStyle w:val=\"FootnoteReference\"/></w:rPr>"
               "<w:footnoteRef/>"
               "</w:r>"
               "<w:r>"
               "<w:t xml:space=\"preserve\"> " + xmlEscape(fn.text) + "</w:t>"
               "</w:r>"
               "</w:p>"
               "</w:footnote>";
    }

    xml += "</w:footnotes>";
    return xml;
}

std::string Document::buildHeaderXml() {
    std::string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
        "<w:hdr xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\""
        " xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\""
        " xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\">";
    // Use section 0 header for single-section backward compatibility
    xml += m_impl->m_sections[0].header
        ? m_impl->m_sections[0].header->toXml() : std::string("<w:p/>");
    xml += "</w:hdr>";
    return xml;
}

std::string Document::buildFooterXml() {
    std::string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
        "<w:ftr xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\""
        " xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\""
        " xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\">";
    xml += m_impl->m_sections[0].footer
        ? m_impl->m_sections[0].footer->toXml() : std::string("<w:p/>");
    xml += "</w:ftr>";
    return xml;
}

std::string Document::buildHeadingNumberingXml() {
    std::string xml;
    xml += "<w:abstractNum w:abstractNumId=\"10\">"
           "<w:multiLevelType w:val=\"hybridMultilevel\"/>";

    // Level 1 text depends on format
    std::string l1Text = (m_impl->m_headingNumFormat == HeadingNumFormat::Chapter)
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
    if (m_impl->m_headingNumbering) {
        xml += buildHeadingNumberingXml();
    }

    // --- Num instances (must come AFTER all abstractNum definitions) ---
    // numId=1: Bullet list (shared by all bullet lists)
    xml += "<w:num w:numId=\"1\">"
           "<w:abstractNumId w:val=\"0\"/>"
           "</w:num>";

    // numId=3+: Each ordered list gets its own restart-from-1
    for (int id = 3; id < m_impl->m_nextOrderedListId; ++id) {
        xml += "<w:num w:numId=\"" + std::to_string(id) + "\">"
               "<w:abstractNumId w:val=\"1\"/>"
               "<w:lvlOverride w:ilvl=\"0\">"
               "<w:startOverride w:val=\"1\"/>"
               "</w:lvlOverride>"
               "</w:num>";
    }

    // numId=10: Heading numbering instance
    if (m_impl->m_headingNumbering) {
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

    for (size_t si = 0; si < m_impl->m_sections.size(); ++si) {
        const auto& sec = m_impl->m_sections[si];
        if (sec.header) {
            xml += "<Override PartName=\"/word/header" + std::to_string(si + 1) + ".xml\" "
                   "ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.header+xml\"/>";
        }
        if (sec.footer) {
            xml += "<Override PartName=\"/word/footer" + std::to_string(si + 1) + ".xml\" "
                   "ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.footer+xml\"/>";
        }
    }
    if (!m_impl->m_footnotes.empty()) {
        xml += "<Override PartName=\"/word/footnotes.xml\" "
               "ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.footnotes+xml\"/>";
    }

    // Image types (document-level)
    for (const auto& img : m_impl->m_images) {
        if (img->skipped()) continue;
        namespace fs = std::filesystem;
        std::string ext = fs::u8path(img->filepath()).extension().u8string();
        if (!ext.empty()) ext = ext.substr(1);
        xml += "<Override PartName=\"/word/" + img->mediaPath() + "\" "
               "ContentType=\"" + imageContentType(ext) + "\"/>";
    }

    // Image types (cell-level from tables)
    for (const auto& table : m_impl->m_tables) {
        for (int r = 0; r < table->rows(); ++r) {
            for (int c = 0; c < table->cols(); ++c) {
                for (const auto& cimg : table->cell(r, c).images()) {
                    if (cimg.skipped) continue;
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
    if (m_impl->m_isTemplate)
        return saveTemplate(filepath);

    try {
        namespace fs = std::filesystem;

        // Probe each image source file. Missing/unreadable files are marked
        // skipped and dropped from the docx so the resulting file is still
        // a valid OPC package that Word can open.
        auto probe = [](const std::string& p) {
            if (p.empty()) return false;
            std::error_code ec;
            return fs::is_regular_file(fs::u8path(p), ec);
        };
        for (auto& img : m_impl->m_images) {
            if (!probe(img->filepath())) {
                img->setSkipped(true);
                std::cerr << "[xword] warning: skipping missing image '"
                          << img->filepath() << "'\n";
            }
        }
        for (auto& table : m_impl->m_tables) {
            for (int r = 0; r < table->rows(); ++r) {
                for (int c = 0; c < table->cols(); ++c) {
                    for (auto& cimg : table->cell(r, c).images()) {
                        if (!probe(cimg.filepath)) {
                            cimg.skipped = true;
                            std::cerr << "[xword] warning: skipping missing image '"
                                      << cimg.filepath << "'\n";
                        }
                    }
                }
            }
        }

        // Collect and register cell images from all tables
        std::vector<CellImage*> cellImages;
        for (auto& table : m_impl->m_tables) {
            table->collectCellImages(cellImages);
        }

        // Assign rIds to cell images (skip dropped ones)
        int imgIndex = static_cast<int>(m_impl->m_images.size());
        int cellRidCounter = 0;
        for (auto* cimg : cellImages) {
            if (cimg->skipped) {
                cimg->rId.clear();
                continue;
            }
            cimg->rId = "rId_img_" + std::to_string(imgIndex + (++cellRidCounter));
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
        if (!m_impl->m_footnotes.empty())
            zip.addEntry("word/footnotes.xml", buildFootnotesXml());

        // Per-section headers and footers
        for (size_t si = 0; si < m_impl->m_sections.size(); ++si) {
            const auto& sec = m_impl->m_sections[si];
            std::string partName = (si == 0) ? "" : std::to_string(si + 1);
            if (sec.header) {
                std::string hdrXml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                    "<w:hdr xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\""
                    " xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\""
                    " xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\">"
                    + sec.header->toXml() + "</w:hdr>";
                zip.addEntry("word/header" + partName + ".xml", hdrXml);
            }
            if (sec.footer) {
                std::string ftrXml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                    "<w:ftr xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\""
                    " xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\""
                    " xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\">"
                    + sec.footer->toXml() + "</w:ftr>";
                zip.addEntry("word/footer" + partName + ".xml", ftrXml);
            }
        }

        // Add document-level images
        for (const auto& img : m_impl->m_images) {
            if (img->skipped()) continue;
            if (!zip.addFileEntry("word/" + img->mediaPath(), img->filepath())) {
                return false;
            }
        }

        // Add cell images from tables
        for (auto* cimg : cellImages) {
            if (cimg->skipped) continue;
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

// ================================================================
//  Template engine helpers
// ================================================================

namespace {

// Trim whitespace from both ends.
std::string trim(const std::string& s) {
    size_t b = 0;
    while (b < s.size() && (s[b] == ' ' || s[b] == '\t' || s[b] == '\r' || s[b] == '\n')) ++b;
    size_t e = s.size();
    while (e > b && (s[e - 1] == ' ' || s[e - 1] == '\t' || s[e - 1] == '\r' || s[e - 1] == '\n')) --e;
    return s.substr(b, e - b);
}

struct Marker {
    size_t pStart = 0;  // <w:p position in body
    size_t pEnd = 0;    // position after </w:p>
    std::string kind;   // "if", "else", "endif"
    std::string key;    // for "if" only
};

// Find all template markers in body XML and return sorted by position.
std::vector<Marker> findMarkers(const std::string& body) {
    std::vector<Marker> out;
    std::string patterns[] = {"{%if ", "{%else%}", "{%endif%}"};
    std::string kinds[]   = {"if",     "else",   "endif"};

    for (int pi = 0; pi < 3; ++pi) {
        const auto& pat = patterns[pi];
        const auto& kind = kinds[pi];

        size_t pos = 0;
        while ((pos = body.find(pat, pos)) != std::string::npos) {
            // Locate enclosing <w:p> — search backwards for the last <w:p
            // that is not a <w:pPr.
            size_t search = pos;
            size_t pStart = std::string::npos;
            while (search > 0) {
                pStart = body.rfind("<w:p", search);
                if (pStart == std::string::npos) break;
                // distinguish <w:p (para) from <w:pPr (para properties)
                if (pStart + 5 <= body.size() && body[pStart + 4] == 'P') {
                    search = pStart - 1;
                    continue; // skip <w:pPr
                }
                break;
            }
            if (pStart == std::string::npos) { pos++; continue; }

            size_t pEnd = body.find("</w:p>", pos);
            if (pEnd == std::string::npos) { pos++; continue; }
            pEnd += 6;

            std::string key;
            if (kind == "if") {
                // {%if  KEY  %}   extract KEY, strip whitespace
                size_t keyBeg = pos + pat.size();
                size_t keyEnd = body.find("%}", keyBeg);
                if (keyEnd != std::string::npos)
                    key = trim(body.substr(keyBeg, keyEnd - keyBeg));
            }

            out.push_back({pStart, pEnd, kind, key});
            pos = pEnd;
        }
    }

    std::sort(out.begin(), out.end(),
        [](const Marker& a, const Marker& b) { return a.pStart < b.pStart; });
    return out;
}

bool isTruthy(const std::string& v) {
    return !v.empty() && v != "false" && v != "0";
}

// Replace ${key} placeholders in a string.
std::string replaceVars(const std::string& s,
     const std::unordered_map<std::string, std::string>& vars)
{
    std::string r = s;
    for (auto it = vars.begin(); it != vars.end(); ++it) {
        std::string ph = "${" + it->first + "}";
        size_t p = 0;
        while ((p = r.find(ph, p)) != std::string::npos) {
            r.replace(p, ph.size(), it->second);
            p += it->second.size();
        }
    }
    return r;
}

} // anonymous namespace

// ================================================================
//  Document template API
// ================================================================

bool Document::open(const std::string& filepath) {
    auto parts = internal::readZip(filepath);
    if (parts.empty()) return false;
    m_impl->m_templateParts = std::move(parts);
    m_impl->m_isTemplate = true;
    m_impl->m_templateVars.clear();
    return true;
}

Document& Document::set(const std::string& key, const std::string& value) {
    m_impl->m_templateVars[key] = value;
    return *this;
}

Document& Document::set(const std::string& key, double v, int precision) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.*f", precision, v);
    return set(key, std::string(buf));
}

std::string Document::renderXml(const std::string& xml) {
    // Locate <w:body> … </w:body>
    size_t bodyTag = xml.find("<w:body>");
    size_t bodyEnd = xml.find("</w:body>");
    if (bodyTag == std::string::npos || bodyEnd == std::string::npos)
        return replaceVars(xml, m_impl->m_templateVars); // simple fallback for headers

    std::string prefix = xml.substr(0, bodyTag + 9);
    std::string body   = xml.substr(bodyTag + 9, bodyEnd - (bodyTag + 9));
    std::string suffix = xml.substr(bodyEnd);

    // 1. Process conditionals
    auto markers = findMarkers(body);

    std::string bodyOut;
    size_t pos = 0;
    enum { Normal, IfTrue, IfFalse } state = Normal;

    for (const auto& m : markers) {
        // Emit content between previous position and this marker
        if (state != IfFalse)
            bodyOut += body.substr(pos, m.pStart - pos);

        if (m.kind == "if") {
            if (state != Normal)
                throw std::runtime_error("nested {%if%} is not supported");
            state = isTruthy(
                m_impl->m_templateVars.count(m.key) ? m_impl->m_templateVars.at(m.key) : "")
                ? IfTrue : IfFalse;
        } else if (m.kind == "else") {
            if (state == Normal)
                throw std::runtime_error("{%else%} without {%if%}");
            state = (state == IfTrue) ? IfFalse : IfTrue;
        } else { // endif
            if (state == Normal)
                throw std::runtime_error("{%endif%} without {%if%}");
            state = Normal;
        }
        pos = m.pEnd;
    }

    if (state != Normal)
        throw std::runtime_error("unclosed {%if%} block");

    if (state != IfFalse)
        bodyOut += body.substr(pos);

    // 2. Replace ${key} variables in surviving XML.
    //    Simple string substitution is safe because ${key} appears only
    //    in <w:t> text content, never in XML tags or attributes.
    bodyOut = replaceVars(bodyOut, m_impl->m_templateVars);

    return prefix + bodyOut + suffix;
}

bool Document::saveTemplate(const std::string& filepath) {
    try {
        ZipWriter zip(filepath);

        // Process document.xml if present
        std::string docXml;
        if (m_impl->m_templateParts.count("word/document.xml")) {
            docXml = m_impl->m_templateParts["word/document.xml"];
            docXml = renderXml(docXml);
        }

        // Write all parts back (explicit iterators — structured bindings
        // may crash on v141 toolset).
        for (auto it = m_impl->m_templateParts.begin(); it != m_impl->m_templateParts.end(); ++it) {
            const std::string& name = it->first;
            std::string& data = it->second;
            if (name == "word/document.xml") {
                zip.addEntry(name, docXml);
            } else if (name.size() > 15 && name.substr(0, 12) == "word/header"
                       && name.substr(name.size() - 4) == ".xml") {
                zip.addEntry(name, renderXml(data));
            } else if (name.size() > 15 && name.substr(0, 12) == "word/footer"
                       && name.substr(name.size() - 4) == ".xml") {
                zip.addEntry(name, renderXml(data));
            } else {
                zip.addEntry(name, data);
            }
        }

        zip.finalize();
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

} // namespace xword
