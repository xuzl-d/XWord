#pragma once

#include "Paragraph.hpp"
#include "Table.hpp"
#include "Image.hpp"
#include "Equation.hpp"
#include "Types.hpp"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <filesystem>

namespace xword {

class BulletList {
public:
    BulletList(ListType type = ListType::Bullet);

    BulletList& addItem(const std::string& text);
    BulletList& setLevel(int level);
    BulletList& setNumId(int id);

    std::string toXml() const;
    int numId() const { return m_numId; }

private:
    ListType m_type;
    int m_level = 0;
    int m_numId = 0; // assigned by Document, 0 = use default
    std::vector<std::string> m_items;
};

class Document {
public:
    Document();
    ~Document();

    Document(const Document&) = delete;
    Document& operator=(const Document&) = delete;

    // ---- Page settings ----
    Document& setPage(const Page& page);
    // Set default first-line indent for body paragraphs (chars, default fontSize 12pt)
    Document& setDefaultParagraphIndent(double chars = 2, int fontSizePt = 12);

    // ---- Heading ----
    Document& addHeading(const std::string& text, int level); // level 1-6
    Document& addHeadingNoNum(const std::string& text, int level); // heading without number
    Document& setHeadingStyle(int level, const HeadingStyle& style);
    Document& enableHeadingNumbering();
    Document& disableHeadingNumbering();
    Document& setHeadingNumFormat(HeadingNumFormat fmt);

    // ---- TOC ----
    Document& addTOC(const std::string& levels = "1-3", const std::string& title = "");

    // ---- Paragraph ----
    Paragraph& addParagraph(const std::string& text = "");

    // ---- Image ----
    // std::string is interpreted as UTF-8; use fs::path or std::wstring for native paths.
    Image& addImage(const std::string& filepath);
    Image& addImage(const char* filepath) { return addImage(std::string(filepath)); }
    Image& addImage(const std::filesystem::path& filepath);
    Image& addImage(const std::wstring& filepath);
    Image& addImage(const wchar_t* filepath) { return addImage(std::wstring(filepath)); }
    Document& enableImageNumbering(const std::string& prefix = "图",
                                   CaptionNumStyle style = CaptionNumStyle::Sequential);
    Document& disableImageNumbering();

    // ---- Table ----
    Table& addTable(int rows, int cols);
    Document& enableTableNumbering(const std::string& prefix = "表",
                                   CaptionNumStyle style = CaptionNumStyle::Sequential);
    Document& disableTableNumbering();

    // ---- Lists ----
    BulletList& addBulletList();
    BulletList& addOrderedList();

    // ---- Equations ----
    Equation& addEquation(const std::string& latex);
    Equation& addDisplayEquation(const std::string& latex);

    // ---- Header / Footer ----
    // Convenience overload: a single centered text line.
    Document& setHeader(const std::string& text);
    Document& setFooter(const std::string& text);
    // Builder overload: returns a Paragraph& so callers can do rich text /
    // page numbers / alignment, e.g.
    //   doc.setFooter().addRun("第 ").addPageNumber()
    //                  .addRun(" 页 / 共 ").addPageCount().addRun(" 页")
    //                  .setAlignment(Alignment::Center);
    Paragraph& setHeader();
    Paragraph& setFooter();
    void clearHeader();
    void clearFooter();

    // ---- Template ----
    // Load a .docx as a template. Its document.xml, header, footer etc. are
    // kept verbatim. Call set() to fill placeholder values, then save() to
    // write the rendered result.
    bool open(const std::string& filepath);

    // Store a template variable.  These replace ${key} placeholders and
    // drive {%if key%} / {%else%} / {%endif%} blocks.
    Document& set(const std::string& key, const std::string& value);
    Document& set(const std::string& key, const char*  value) { return set(key, std::string(value)); }
    Document& set(const std::string& key, bool   v) { return set(key, std::string(v ? "true" : "false")); }
    Document& set(const std::string& key, int    v) { return set(key, std::to_string(v)); }
    Document& set(const std::string& key, double v, int precision = 2);
    template<typename T> Document& set(const std::string& key, T) = delete; // reject unsupported types

    // ---- Save ----
    bool save(const std::string& filepath);

private:
    enum class ElementType { Heading, Paragraph, Image, Table, BulletList, Equation, TOC };

    struct Element {
        ElementType type;
        int headingLevel = 0;
        std::string text;
        std::string extra;     // for TOC: level range
        bool noNumbering = false;
        union Data {
            Paragraph* paragraph = nullptr;
            Image* image;
            Table* table;
            BulletList* bulletList;
            Equation* equation;
            void* ptr;
            Data() {}
        } data;
    };

    void buildDocumentXml(std::string& xml);
    std::string buildRelationshipsXml();
    std::string buildContentTypesXml();
    std::string buildNumberingXml();
    std::string buildStylesXml();
    std::string buildHeadingNumberingXml();
    std::string buildHeaderXml();
    std::string buildFooterXml();

    Page m_page;
    std::vector<std::unique_ptr<Paragraph>> m_paragraphs;
    std::vector<std::unique_ptr<Image>> m_images;
    std::vector<std::unique_ptr<Table>> m_tables;
    std::vector<std::unique_ptr<BulletList>> m_lists;
    std::vector<std::unique_ptr<Equation>> m_equations;

    std::vector<Element> m_elements;

    HeadingStyle m_headingStyles[6];
    bool m_headingNumbering = false;
    HeadingNumFormat m_headingNumFormat = HeadingNumFormat::Decimal;
    int m_nextOrderedListId = 3;
    int m_defaultIndent = 480;
    bool m_imageNumbering = false;
    std::string m_imageNumPrefix = "图";
    CaptionNumStyle m_imageNumStyle = CaptionNumStyle::Sequential;
    bool m_tableNumbering = false;
    std::string m_tableNumPrefix = "表";
    CaptionNumStyle m_tableNumStyle = CaptionNumStyle::Sequential;
    std::unique_ptr<Paragraph> m_header;
    std::unique_ptr<Paragraph> m_footer;

    // Template state
    bool m_isTemplate = false;
    std::unordered_map<std::string, std::string> m_templateParts;    // zip entry → data
    std::unordered_map<std::string, std::string> m_templateVars;

    std::string renderXml(const std::string& xml);
    bool saveTemplate(const std::string& filepath);
};

} // namespace xword
