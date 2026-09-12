#pragma once

#include <array>

#include "Content.hpp"

namespace xword
{

/// One column of a multi-column section.
struct Column
{
    Length width; ///< Column width.
    Length space; ///< Space after the column.
};

/// Page setup and header/footer stories for one section of the document.
///
/// Sections are created through Document::addSectionBreak() / addSection();
/// the first one exists automatically.  A new section inherits the previous
/// section's layout, so only the differences need to be set.
class Section
{
public:
    /// @param type  How this section starts relative to the previous one.
    explicit Section(SectionBreakType type = SectionBreakType::NextPage);

    /// @name Rule of five
    /// A section owns its page settings and header/footer stories: movable,
    /// not copyable.
    /// @{
    ~Section();
    Section(const Section&)            = delete;
    Section& operator=(const Section&) = delete;
    /// @}

    /// Set page size, orientation, margins and distances.
    Section& setPage(const Page& page);

    /// The page settings in use.
    const Page& page() const;

    /// Set equal-width columns.
    /// @param count      Number of columns, 1..45.
    /// @param space      Space between columns (default 36 pt).
    /// @param separator  Draw a line between columns.
    Section& setColumns(int count, Length space = Length::pt(36), bool separator = false);

    /// Set columns with explicit widths.
    Section& setColumnWidths(const std::vector<Column>& columns, bool separator = false);

    /// Page number format and starting number.
    /// @param start  0 keeps the previous section's numbering.
    Section& setPageNumbering(NumberFormat format = NumberFormat::Decimal, int start = 0);

    /// Give this section a different first page header/footer.
    Section& setTitlePage(bool on = true);

    /// Set the East-Asian document grid.  A line grid makes Word lay text out
    /// on a fixed line pitch, which is how Chinese-locale documents are
    /// normally laid out (Word's own default is Lines with 312 twips).
    /// @param linePitch Line pitch in twips (312 twips = 15.6 pt).
    Section& setDocumentGrid(DocGridType type = DocGridType::Lines, int linePitch = 312);

    /// Remove the document grid (lines take their natural height).
    Section& clearDocumentGrid();

    /// Footnote numbering options for this section.
    Section& setFootnoteOptions(const NoteOptions& options);

    /// Endnote numbering options for this section.
    Section& setEndnoteOptions(const NoteOptions& options);

    /// @{
    /// Access a header or footer story, creating it if needed.
    Content& header(HeaderFooterType type = HeaderFooterType::Default);
    Content& footer(HeaderFooterType type = HeaderFooterType::Default);
    /// @}

    /// @{
    /// Drop a header or footer so it continues the previous section's.
    Section& linkHeaderToPrevious(HeaderFooterType type = HeaderFooterType::Default);
    Section& linkFooterToPrevious(HeaderFooterType type = HeaderFooterType::Default);
    /// @}

    /// @{
    /// Replace a header or footer with empty content.
    Section& clearHeader(HeaderFooterType type = HeaderFooterType::Default);
    Section& clearFooter(HeaderFooterType type = HeaderFooterType::Default);
    /// @}

    /// @{
    /// Internal read access used by the package writer.
    const Content* headerContent(HeaderFooterType type) const;
    const Content* footerContent(HeaderFooterType type) const;
    /// @}

    /// Build the `<w:sectPr>` fragment (internal use).
    std::string propertiesXml() const;

    /// Usable width between the margins, in twips.
    int contentWidth() const;

    /// Usable width of one column, in twips.
    int columnWidth() const;

    /// Copy the layout settings (not the content) from a previous section.
    void inheritLayout(const Section& previous);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};
} // namespace xword
