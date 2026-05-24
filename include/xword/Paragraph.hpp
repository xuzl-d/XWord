#pragma once

#include "Run.hpp"
#include "Types.hpp"
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
    /// @}

    /// Build OOXML paragraph XML (internal use).
    std::string toXml() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace xword
