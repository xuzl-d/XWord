#pragma once

#include "Run.hpp"
#include "Types.hpp"
#include "Style.hpp"
#include "Image.hpp"
#include <memory>
#include <string>

namespace xword {

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
///
class Paragraph {
public:
    Paragraph();
    ~Paragraph();
    Paragraph(Paragraph&&) noexcept;
    Paragraph& operator=(Paragraph&&) noexcept;

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
    Paragraph& addEndnoteRef(int id);
    Paragraph& addSectionPageCount();
    Paragraph& addBreak(BreakType type = BreakType::Line);
    Paragraph& addTab();
    Paragraph& addField(const std::string& instruction, const std::string& cached = "");
    Paragraph& addHyperlink(const std::string& text, const std::string& url, const RunStyle& style = RunStyle());
    Paragraph& startBookmark(const TargetId& target);
    Paragraph& endBookmark(const TargetId& target);
    Paragraph& setBookmark(const TargetId& target);
    Paragraph& addReference(const TargetId& target, ReferenceKind kind = ReferenceKind::Text, bool hyperlink = true);
    Paragraph& addCitation(const std::string& tag, const std::string& pages = "");
    Paragraph& startComment(int id);
    Paragraph& endComment(int id);
    Image& addImage(const std::string& path);
    Paragraph& setStyleId(const std::string& id);
    Paragraph& setParagraphStyle(const ParagraphStyle& style);
    Paragraph& setPageBreakBefore(bool on = true);
    Paragraph& setKeepWithNext(bool on = true);
    Paragraph& setKeepTogether(bool on = true);
    Paragraph& setWidowControl(bool on = true);
    Paragraph& setLeftIndent(Length n);
    Paragraph& setRightIndent(Length n);
    Paragraph& setHangingIndent(Length n);
    Paragraph& setLineSpacing(double n, LineRule rule = LineRule::Auto);
    Paragraph& addTabStop(const TabStop& tab);
    Paragraph& setBorder(const Border& border);
    Paragraph& setShading(const std::string& color);
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

    RunStyle& getStyle();

    /// Build OOXML paragraph XML (internal use).
    std::string toXml() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace xword
