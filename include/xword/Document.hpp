#pragma once

#include "Paragraph.hpp"
#include "Table.hpp"
#include "BulletList.hpp"
#include "Image.hpp"
#include "Equation.hpp"
#include "Types.hpp"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>

namespace xword {

// ════════════════════════════════════════════════════════════
//  Document  —  top-level docx document builder
// ════════════════════════════════════════════════════════════

/// The central class for building a .docx document.
///
/// ## Programmatic mode
///
/// Build a document from scratch with a fluent API:
///
///     Document doc;
///     doc.setPage(Page().setSize(PageSize::A4));
///     doc.addHeading("Title", 1);
///     doc.addParagraph("Hello world.");
///     doc.addTable(3, 2).setStyle(TableStyle::Grid);
///     doc.save("output.docx");
///
/// ## Template mode
///
/// Load a .docx template, fill placeholders, and save the result:
///
///     Document doc;
///     doc.open("template.docx");
///     doc.set("name", "Zhang San");
///     doc.set("amount", 128.50);
///     doc.save("output.docx");
///
/// Template syntax:
///   - `${key}`          — variable placeholder (replaced by set() value)
///   - `{%if key%}`      — conditional block start (standalone paragraph)
///   - `{%else%}`        — else branch (standalone paragraph, optional)
///   - `{%endif%}`       — conditional block end (standalone paragraph)
///
/// Condition keys are truthy unless the value is "false", "0", or "".
///
class Document {
public:
    Document();
    ~Document();

    Document(const Document&) = delete;
    Document& operator=(const Document&) = delete;

    // ── Page settings ──────────────────────────────────────

    /// Set page size, orientation, and margins.
    Document& setPage(const Page& page);

    /// Set default first-line indent for body paragraphs.
    /// @param chars       Number of characters to indent.
    /// @param fontSizePt  Reference font size in pt (default 12).
    Document& setDefaultParagraphIndent(double chars = 2, int fontSizePt = 12);

    // ── Headings ───────────────────────────────────────────

    /// Add a numbered heading (level 1–6).
    Document& addHeading(const std::string& text, int level);
    Document& addHeading(const std::wstring& text, int level);

    /// Add a heading excluded from auto-numbering.
    Document& addHeadingNoNum(const std::string& text, int level);
    Document& addHeadingNoNum(const std::wstring& text, int level);

    /// Customise heading style for a given level.
    Document& setHeadingStyle(int level, const HeadingStyle& style);

    /// Enable automatic heading numbering.
    Document& enableHeadingNumbering();

    /// Disable automatic heading numbering.
    Document& disableHeadingNumbering();

    /// Set heading numbering format (Decimal or Chapter).
    Document& setHeadingNumFormat(HeadingNumFormat fmt);

    // ── Table of Contents ──────────────────────────────────

    /// Insert a TOC field.
    /// @param levels  Outline level range, e.g. "1-3".
    /// @param title   Optional TOC heading; if non-empty a heading is
    ///                prepended (without numbering).
    Document& addTOC(const std::string& levels = "1-3",
                     const std::string& title = "");
    Document& addTOC(const std::wstring& levels, const std::wstring& title);

    // ── Paragraphs ─────────────────────────────────────────

    /// Add a body paragraph.
    /// @return Reference for chaining (addRun, setAlignment, …).
    Paragraph& addParagraph(const std::string& text = "");
    Paragraph& addParagraph(const std::wstring& text);

    // ── Images ─────────────────────────────────────────────

    /// Add an image.
    /// @return Reference for chaining (setSize, setCaption, …).
    Image& addImage(const std::string& filepath);

    /// @{
    /// Convenience overloads for various path types.
    Image& addImage(const char* filepath) { return addImage(std::string(filepath)); }
    Image& addImage(const std::filesystem::path& filepath);
    Image& addImage(const std::wstring& filepath);
    Image& addImage(const wchar_t* filepath) { return addImage(std::wstring(filepath)); }
    /// @}

    /// Enable automatic image caption numbering.
    /// @param prefix  Caption prefix, e.g. "Fig." or "图".
    /// @param style   Numbering strategy (Sequential or ByChapter).
    Document& enableImageNumbering(
        const std::string& prefix = "\xe5\x9b\xbe",
        CaptionNumStyle style = CaptionNumStyle::Sequential);

    /// Disable image caption numbering.
    Document& disableImageNumbering();

    // ── Tables ─────────────────────────────────────────────

    /// Add a table with the given dimensions.
    /// @return Reference for chaining (setStyle, setCaption, …).
    Table& addTable(int rows, int cols);

    /// Enable automatic table caption numbering.
    Document& enableTableNumbering(
        const std::string& prefix = "\xe8\xa1\xa8",
        CaptionNumStyle style = CaptionNumStyle::Sequential);

    /// Disable table caption numbering.
    Document& disableTableNumbering();

    // ── Lists ──────────────────────────────────────────────

    /// Add a bullet (unordered) list.
    BulletList& addBulletList();

    /// Add an ordered (numbered) list.
    BulletList& addOrderedList();

    // ── Sections ──────────────────────────────────────────

    /// Insert a section break. Elements after this call belong to a new
    /// section that inherits the current section's page settings.
    /// Call setPage() / setHeader() / setFooter() afterwards to customise
    /// the new section.
    Document& addSectionBreak(SectionBreakType type = SectionBreakType::NextPage);

    /// Enable different first page header/footer for the current section.
    Document& enableTitlePage();

    // ── Footnotes ─────────────────────────────────────────

    /// Add a footnote with the given text and return its ID.
    /// Use Paragraph::addFootnoteRef(id) to place the reference mark in text.
    int addFootnote(const std::string& text);

    // ── Equations ──────────────────────────────────────────

    /// Add an inline LaTeX equation (rendered as OMML).
    Equation& addEquation(const std::string& latex);

    /// Add a display-style LaTeX equation (centred, on its own line).
    Equation& addDisplayEquation(const std::string& latex);

    // ── Header / Footer ───────────────────────────────────

    /// Set a centred single-line page header.
    Document& setHeader(const std::string& text);

    /// Set a centred single-line page footer.
    Document& setFooter(const std::string& text);

    /// Builder overload: returns a Paragraph& for rich header content.
    /// Example:
    ///   doc.setHeader().addRun("Chapter ").addPageNumber()
    ///                  .setAlignment(Alignment::Center);
    Paragraph& setHeader();

    /// Builder overload: returns a Paragraph& for rich footer content.
    Paragraph& setFooter();

    /// Remove the page header.
    void clearHeader();

    /// Remove the page footer.
    void clearFooter();

    // ── Template engine ────────────────────────────────────

    /// Load a .docx file as a template.
    /// @return false if the file could not be opened.
    bool open(const std::string& filepath);

    /// Store a template variable value.
    Document& set(const std::string& key, const std::string& value);

    /// @{
    /// Convenience overloads for common types.
    Document& set(const std::string& key, const char* v) { return set(key, std::string(v)); }
    Document& set(const std::string& key, bool   v) { return set(key, std::string(v ? "true" : "false")); }
    Document& set(const std::string& key, int    v) { return set(key, std::to_string(v)); }
    /// Store a double with a specified number of decimal places.
    Document& set(const std::string& key, double v, int precision = 2);
    /// @}

    /// Prevent accidental implicit conversions (linker error if used).
    template<typename T> Document& set(const std::string& key, T) = delete;

    // ── Save ───────────────────────────────────────────────

    /// Write the document to a .docx file.
    /// @return true on success.
    bool save(const std::string& filepath);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;

    // ── Internal build helpers ──────────────────
    void        buildDocumentXml(std::string& xml);
    std::string buildRelationshipsXml();
    std::string buildContentTypesXml();
    std::string buildNumberingXml();
    std::string buildStylesXml();
    std::string buildHeadingNumberingXml();
    std::string buildFootnotesXml();
    std::string buildHeaderXml();
    std::string buildFooterXml();
    std::string renderXml(const std::string& xml);
    bool        saveTemplate(const std::string& filepath);
};

} // namespace xword
