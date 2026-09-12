#pragma once
#include "Content.hpp"
#include <array>
namespace xword {
struct Column { Length width, space; };
class Section {
public:
    explicit Section(SectionBreakType type = SectionBreakType::NextPage);
    ~Section();
    Section(const Section&) = delete; Section& operator=(const Section&) = delete;
    Section& setPage(const Page& page);
    const Page& page() const;
    Section& setColumns(int count,Length space = Length::pt(36),bool separator = false);
    Section& setColumnWidths(const std::vector<Column>& columns,bool separator = false);
    Section& setPageNumbering(NumberFormat format = NumberFormat::Decimal,int start = 0);
    /// Set the East-Asian document grid.  A line grid makes Word lay text out
    /// on a fixed line pitch, which is how Chinese-locale documents are
    /// normally laid out (Word's own default is Lines with 312 twips).
    /// @param linePitch Line pitch in twips (312 twips = 15.6 pt).
    Section& setDocumentGrid(DocGridType type = DocGridType::Lines,int linePitch = 312);
    /// Remove the document grid (lines take their natural height).
    Section& clearDocumentGrid();
    Section& setTitlePage(bool on = true);
    Section& setFootnoteOptions(const NoteOptions& options);
    Section& setEndnoteOptions(const NoteOptions& options);
    Content& header(HeaderFooterType type = HeaderFooterType::Default);
    Content& footer(HeaderFooterType type = HeaderFooterType::Default);
    Section& linkHeaderToPrevious(HeaderFooterType type = HeaderFooterType::Default);
    Section& linkFooterToPrevious(HeaderFooterType type = HeaderFooterType::Default);
    Section& clearHeader(HeaderFooterType type = HeaderFooterType::Default);
    Section& clearFooter(HeaderFooterType type = HeaderFooterType::Default);
    /// Internal read access used by the package writer.
    const Content* headerContent(HeaderFooterType type) const;
    const Content* footerContent(HeaderFooterType type) const;
    std::string propertiesXml() const;
    int contentWidth() const;
    int columnWidth() const;
    void inheritLayout(const Section& previous);
private:
    struct Impl; std::unique_ptr<Impl> m_impl;
};
}
