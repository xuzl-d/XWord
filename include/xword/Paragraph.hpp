#pragma once

#include <memory>
#include <string>

#include "Image.hpp"
#include "Run.hpp"
#include "Style.hpp"
#include "Types.hpp"

namespace xword
{

/// A document paragraph composed of text runs, equations, and fields.
///
/// Paragraphs are created through Document::addParagraph() or
/// Table::cell().addParagraph().  The returned reference supports
/// method chaining for building rich content inline:
///
///     doc.addParagraph()
///        .addRun("Bold ", RunStyle().bold())
///        .addRun("Normal")
///        .setAlignment(Alignment::Center);
class Paragraph
{
public:
    /// Create an empty paragraph.
    Paragraph();

    /// @name Rule of five
    /// A paragraph owns its runs and images: movable, not copyable.
    /// @{
    ~Paragraph();
    Paragraph(Paragraph&&) noexcept;
    Paragraph& operator=(Paragraph&&) noexcept;
    /// @}

    /// @{
    /// Add content runs (chainable).

    /// Add a plain-text run.
    Paragraph& addRun(const std::string& text);
    Paragraph& addRun(const std::wstring& text);

    /// Add a styled text run.
    Paragraph& addRun(const std::string& text, const RunStyle& style);
    Paragraph& addRun(const std::wstring& text, const RunStyle& style);

    /// Add an inline LaTeX equation run.
    Paragraph& addEquation(const std::string& latex);
    Paragraph& addEquation(const std::wstring& latex);

    /// Insert a Word `PAGE` field (current page number).
    Paragraph& addPageNumber();

    /// Insert a Word `NUMPAGES` field (total page count).
    Paragraph& addPageCount();

    /// Insert a footnote reference mark (superscript number linking to
    /// the footnote whose content was added via Document::addFootnote).
    Paragraph& addFootnoteRef(int footnoteId);

    /// Insert an endnote reference mark.
    Paragraph& addEndnoteRef(int id);

    /// Insert a field returning the number of pages in the current section.
    Paragraph& addSectionPageCount();

    /// Insert a break (line, page, or column).
    Paragraph& addBreak(BreakType type = BreakType::Line);

    /// Insert a tab character.
    Paragraph& addTab();

    /// Insert an arbitrary Word field with a cached result.
    /// @param instruction  Field instruction, e.g. "SEQ Figure \\* ARABIC".
    /// @param cached       Text shown until the field is updated.
    Paragraph& addField(const std::string& instruction, const std::string& cached = "");

    /// Add a hyperlink run.
    /// @param url  External target, or "#bookmark" for an internal link.
    Paragraph& addHyperlink(const std::string& text, const std::string& url,
                            const RunStyle& style = RunStyle());

    /// Mark the start of a bookmark.
    Paragraph& startBookmark(const TargetId& target);

    /// Mark the end of a bookmark.
    Paragraph& endBookmark(const TargetId& target);

    /// Mark this whole paragraph as a bookmark target.
    Paragraph& setBookmark(const TargetId& target);

    /// Insert a cross-reference to a bookmark created earlier.
    /// @param kind       Reference the text, the number, or the page.
    /// @param hyperlink  Make the reference clickable.
    Paragraph& addReference(const TargetId& target, ReferenceKind kind = ReferenceKind::Text,
                            bool hyperlink = true);

    /// Insert a citation to a bibliography source.
    /// @param tag    Source tag passed to Document::addSource().
    /// @param pages  Optional page locator.
    Paragraph& addCitation(const std::string& tag, const std::string& pages = "");

    /// @{
    /// Comment range control.  The comment itself is created with
    /// Document::addComment(), which returns the ID used here.
    Paragraph& startComment(int id);
    Paragraph& endComment(int id);
    /// @}

    /// Add an inline image to this paragraph.
    /// @return The image, for setting size or alt text.
    Image& addImage(const std::string& path);

    /// Apply a paragraph style by style ID.
    Paragraph& setStyleId(const std::string& id);

    /// Apply a paragraph style definition directly.
    Paragraph& setParagraphStyle(const ParagraphStyle& style);

    /// Start this paragraph on a new page.
    Paragraph& setPageBreakBefore(bool on = true);

    /// Keep this paragraph on the same page as the next one.
    Paragraph& setKeepWithNext(bool on = true);

    /// Keep all lines of this paragraph on one page.
    Paragraph& setKeepTogether(bool on = true);

    /// Enable or disable widows/orphans control.
    Paragraph& setWidowControl(bool on = true);

    /// Left indent in twips.
    Paragraph& setLeftIndent(Length n);

    /// Right indent in twips.
    Paragraph& setRightIndent(Length n);

    /// Hanging indent in twips (overrides the first-line indent).
    Paragraph& setHangingIndent(Length n);

    /// Line spacing.
    /// @param n     Multiplier for Auto, or a height for the other rules.
    /// @param rule  How @p n is interpreted.
    Paragraph& setLineSpacing(double n, LineRule rule = LineRule::Auto);

    /// Add a tab stop.
    Paragraph& addTabStop(const TabStop& tab);

    /// Draw a border around the paragraph.
    Paragraph& setBorder(const Border& border);

    /// Fill the paragraph background with a hex RRGGBB colour.
    Paragraph& setShading(const std::string& color);

    /// Attach this paragraph to a numbering instance.
    /// @param id     Numbering ID; 0 applies no numbering.
    /// @param level  Nesting level, 0..8.
    Paragraph& setNumbering(int id, int level = 0);
    /// @}

    /// @{
    /// Paragraph formatting (chainable).

    /// Horizontal alignment.
    Paragraph& setAlignment(Alignment align);

    /// First-line indent in twips (1 pt = 20 twips).
    Paragraph& setFirstLineIndent(int twips);

    /// First-line indent in characters (default font size 12 pt).
    Paragraph& setFirstLineIndentChars(double chars, int fontSizePt = 12);

    /// Spacing after paragraph in twips (-1 = inherit from style).
    Paragraph& setSpacingAfter(int twips);

    /// Spacing before paragraph in twips (-1 = inherit from style).
    Paragraph& setSpacingBefore(int twips);

    /// Set the default RunStyle for this paragraph.
    /// Applied to all text runs and inline equations that do not specify
    /// their own style. Document::addParagraph populates this from the
    /// document's body run style.
    Paragraph& setStyle(const RunStyle& style);
    /// @}

    /// The paragraph's default run style (mutable for chaining).
    RunStyle& getStyle();

    /// Build OOXML paragraph XML (internal use).
    std::string toXml() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace xword
