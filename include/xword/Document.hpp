#pragma once

#include "BulletList.hpp"
#include "Equation.hpp"
#include "Image.hpp"
#include "Paragraph.hpp"
#include "Section.hpp"
#include "Table.hpp"
#include "Types.hpp"
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace xword
{

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
///     doc.addTable(3, 2).setBorderStyle(TableStyle::Grid);
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
///   - `${key}`          — variable placeholder (scalar or block via set())
///   - `{%if key%}`      — conditional block start (standalone paragraph)
///   - `{%else%}`        — else branch (standalone paragraph, optional)
///   - `{%endif%}`       — conditional block end (standalone paragraph)
///
/// Scalar `set(key, string)` replaces `${key}` in-text.  Block overloads
/// (`set(key, Table)`, `setParagraph`, `setTable`, `setImage`, …) replace
/// the placeholder's entire paragraph — keep `${key}` on its own line.
/// Condition keys are truthy unless the value is "false", "0", or ""
/// (block content for a key is always truthy).
///
class Document
{
public:
    /// Create an empty document with a single section.
    Document();

    /// @name Rule of five
    /// A document owns every section and every block added to it: movable,
    /// not copyable.
    /// @{
    ~Document();
    Document(const Document&)            = delete;
    Document& operator=(const Document&) = delete;
    /// @}

    // ── Page settings ──────────────────────────────────────

    /// Set page size, orientation, and margins.
    Document& setPage(const Page& page);

    /// Set default first-line indent for body paragraphs.
    /// @param chars       Number of characters to indent.
    /// @param fontSizePt  Reference font size in pt (default 12).
    Document& setDefaultParagraphIndent(double chars = 2, int fontSizePt = 12);

    /// Set default body text font (applied to Normal style).
    /// @param eastAsia  Font for East-Asian (CJK) text.
    /// @param ascii     Font for ASCII/Latin text.
    /// @param hAnsi     Font for high-ANSI text (defaults to eastAsia if empty).
    Document& setBodyFont(const std::string& eastAsia, const std::string& ascii = "",
                          const std::string& hAnsi = "");

    /// Set default body text font size in points.
    Document& setBodyFontSize(double pt);

    /// Set default line spacing multiplier for body text (e.g. 1.25).
    Document& setBodyLineSpacing(double line);

    /// Set the default RunStyle applied to all body paragraphs.
    /// Controls font size, color, font name for text runs in body paragraphs.
    /// Individual addRun() calls with an explicit RunStyle override this default.
    Document& setBodyRunStyle(const RunStyle& style);

    /// Set the default RunStyle for display equations (行间公式).
    /// Applied automatically to every addDisplayEquation() call.
    Document& setDisplayEquationStyle(const RunStyle& style);

    /// Set the default RunStyle for table cell content.
    /// Applied automatically to every Cell::addParagraph() call.
    Document& setTableRunStyle(const RunStyle& style);

    // ── Headings ───────────────────────────────────────────

    /// Add a numbered heading (level 1–9).
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
    Document& addTOC(const std::string& levels = "1-3", const std::string& title = "");
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
    Image& addImage(const char* filepath);
    Image& addImage(const std::filesystem::path& filepath);
    Image& addImage(const std::wstring& filepath);
    Image& addImage(const wchar_t* filepath);
    /// @}

    /// Enable automatic image caption numbering.
    /// @param prefix  Caption prefix, e.g. "Fig." or "图".
    /// @param style   Numbering strategy (Sequential or ByChapter).
    Document& enableImageNumbering(const std::string& prefix = "\xe5\x9b\xbe",
                                   CaptionNumStyle    style  = CaptionNumStyle::Sequential);

    /// Disable image caption numbering.
    Document& disableImageNumbering();

    // ── Tables ─────────────────────────────────────────────

    /// Add a table with the given dimensions.
    /// @return Reference for chaining (setStyle, setCaption, …).
    Table& addTable(int rows, int cols);

    /// Enable automatic table caption numbering.
    Document& enableTableNumbering(const std::string& prefix = "\xe8\xa1\xa8",
                                   CaptionNumStyle    style  = CaptionNumStyle::Sequential);

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

    /// The section that subsequent content is added to.
    Section& currentSection();

    /// Append a new section and make it current.  Returns the new section so
    /// its page settings and header/footer can be configured.
    Section& addSection(SectionBreakType type = SectionBreakType::NextPage);

    /// Insert a page break.
    Document& addPageBreak();

    /// Use separate headers/footers for odd and even pages.
    Document& setEvenAndOddHeaders(bool on = true);

    /// Add a heading, returning its paragraph for extra content.
    /// @param numbered  False to leave the heading out of the numbering sequence.
    Paragraph& addHeadingParagraph(const std::string& text, int level, bool numbered = true);

    /// Insert a table-of-figures field.
    /// @param title  Optional heading placed above the field.
    Document& addFigureTOC(const std::string& title = "");

    /// Insert a table-of-tables field.
    /// @param title  Optional heading placed above the field.
    Document& addTableTOC(const std::string& title = "");

    /// Define a paragraph style that content can then reference by ID.
    Document& registerParagraphStyle(const std::string& id, const ParagraphStyle& style);

    /// Define a character style.
    /// @param basedOn  Style ID to inherit from.
    Document& registerCharacterStyle(const std::string& id, const RunStyle& style,
                                     const std::string& basedOn = "");

    /// Define a table style that tables can then reference by ID.
    Document& registerTableStyle(const std::string& id, const TableStyleDefinition& style);

    /// Add an empty footnote and return it for content and options.
    Note& addFootnote();

    /// Add an endnote holding one paragraph of text; returns its ID.
    int addEndnote(const std::string& text);

    /// Add an empty endnote and return it for content and options.
    Note& addEndnote();

    /// @{
    /// Look up a note by ID.
    Note& footnote(int id);
    Note& endnote(int id);
    /// @}

    /// Add a comment; returns its ID for Paragraph::startComment()/endComment().
    /// @param date  UTC ISO 8601; defaults to the current time.
    int addComment(const std::string& text, const std::string& author,
                   const std::string& date = "");

    /// Register a bibliography entry, cited by tag from Paragraph::addCitation().
    Document& addSource(const BibliographySource& source);

    /// Select the citation style used by the bibliography field.
    Document& setBibliographyStyle(const std::string& style = "IEEE");

    /// Insert a bibliography field.
    /// @param title  Optional heading placed above the field.
    Document& addBibliography(const std::string& title = "");

    /// Set the built-in document properties (title, author, …).
    Document& setProperties(const DocumentProperties& properties);

    /// Set a custom document property.
    Document& setCustomProperty(const std::string& name, const CustomProperty& property);

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
    /// UTF-8 `std::string` paths and Windows wide paths are both accepted.
    bool open(const std::string& filepath);
    bool open(const std::wstring& filepath);
    bool open(const wchar_t* filepath);

    /// Store a template variable value (in-text replacement of `${key}`).
    Document& set(const std::string& key, const std::string& value);

    /// @{
    /// Convenience overloads for common types.
    Document& set(const std::string& key, const char* v);
    Document& set(const std::string& key, bool v);
    Document& set(const std::string& key, int v);
    /// Store a double with a specified number of decimal places.
    Document& set(const std::string& key, double v, int precision = 2);
    /// @}

    /// @{
    /// Replace `${key}` with block-level content (paragraph, table, image,
    /// list, or equation).  The placeholder's enclosing paragraph is replaced
    /// wholesale — put `${key}` on its own line in the template.
    /// Repeated calls for the same key append additional blocks.
    Document& set(const std::string& key, Paragraph para);
    Document& set(const std::string& key, Table table);
    Document& set(const std::string& key, Image image);
    Document& set(const std::string& key, BulletList list);
    Document& set(const std::string& key, Equation eq);
    /// @}

    /// @{
    /// Factory overloads: create block content owned by the document and
    /// return a reference for chaining (same style as addParagraph / addTable).
    Paragraph&  setParagraph(const std::string& key, const std::string& text = "");
    Table&      setTable(const std::string& key, int rows, int cols);
    Image&      setImage(const std::string& key, const std::string& filepath);
    BulletList& setBulletList(const std::string& key);
    BulletList& setOrderedList(const std::string& key);
    Equation&   setEquation(const std::string& key, const std::string& latex);
    Equation&   setDisplayEquation(const std::string& key, const std::string& latex);
    /// @}

    /// Prevent accidental implicit conversions (linker error if used).
    /// Must stay in the header: the deleted overload only suppresses implicit
    /// conversions while it is visible at the call site.
    template <typename T> Document& set(const std::string& key, T) = delete;

    // ── Save ───────────────────────────────────────────────

    /// Write the document to a .docx file.
    /// @return true on success.
    /// UTF-8 `std::string` paths and Windows wide paths are both accepted.
    bool       save(const std::string& filepath);
    bool       save(const std::wstring& filepath);
    bool       save(const wchar_t* filepath);
    SaveResult saveDetailed(const std::string& filepath,
                            const SaveOptions& options = SaveOptions());
    SaveResult saveDetailed(const std::wstring& filepath,
                            const SaveOptions&  options = SaveOptions());
    SaveResult saveDetailed(const wchar_t* filepath, const SaveOptions& options = SaveOptions());

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;

    // ── Internal build helpers ──────────────────
    std::string buildNumberingXml();
    std::string buildStylesXml();
    std::string renderXml(const std::string& xml);
};

} // namespace xword
