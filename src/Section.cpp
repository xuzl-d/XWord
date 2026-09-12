#include "xword/Section.hpp"
#include "internal/ZipWriter.hpp"
#include <algorithm>
namespace xword {
struct Section::Impl {
    Page page; SectionBreakType type; int columns=1,start=0; Length space=Length::pt(36); bool separator=false,title=false;
    DocGridType grid=DocGridType::Default; int gridPitch=312;
    NumberFormat pageFormat=NumberFormat::Decimal; std::vector<Column> widths;
    std::array<std::unique_ptr<Content>,3> headers,footers;
    std::vector<std::unique_ptr<Content>> retired;
    std::optional<NoteOptions> footnotes,endnotes;
};
Section::Section(SectionBreakType t):m_impl(std::make_unique<Impl>()) {m_impl->type=t;}
Section::~Section()=default;
Section& Section::setPage(const Page& p) { m_impl->page=p; return *this; }
const Page& Section::page() const { return m_impl->page; }
Section& Section::setColumns(int n,Length space,bool separator) { if(n<1||n>45||space.dxa()<0) throw std::invalid_argument("Invalid columns"); m_impl->columns=n; m_impl->space=space; m_impl->separator=separator; m_impl->widths.clear(); return *this; }
Section& Section::setColumnWidths(const std::vector<Column>& cols,bool separator) { if(cols.empty()||cols.size()>45) throw std::invalid_argument("Invalid columns"); for(const auto& c:cols) if(c.width.dxa()<=0||c.space.dxa()<0) throw std::invalid_argument("Invalid column dimensions"); m_impl->widths=cols; m_impl->columns=static_cast<int>(cols.size()); m_impl->separator=separator; return *this; }
Section& Section::setPageNumbering(NumberFormat f,int start) { if(start<0||f==NumberFormat::Bullet) throw std::invalid_argument("Invalid page numbering"); m_impl->pageFormat=f; m_impl->start=start; return *this; }
Section& Section::setTitlePage(bool on) { m_impl->title=on; return *this; }
Section& Section::setDocumentGrid(DocGridType t,int linePitch) {
    if(linePitch<=0||linePitch>31680) throw std::invalid_argument("Invalid document grid pitch");
    m_impl->grid=t; m_impl->gridPitch=linePitch; return *this;
}
Section& Section::clearDocumentGrid() { m_impl->grid=DocGridType::Default; return *this; }
Section& Section::setFootnoteOptions(const NoteOptions& o) { if(o.start<1||o.format==NumberFormat::Bullet) throw std::invalid_argument("Invalid footnote options"); m_impl->footnotes=o; return *this; }
Section& Section::setEndnoteOptions(const NoteOptions& o) { if(o.start<1||o.restart==NoteRestart::EachPage||o.format==NumberFormat::Bullet) throw std::invalid_argument("Invalid endnote options"); m_impl->endnotes=o; return *this; }
Content& Section::header(HeaderFooterType t) { auto& p=m_impl->headers.at(static_cast<int>(t)); if(!p)p=std::make_unique<Content>(); return *p; }
Content& Section::footer(HeaderFooterType t) { auto& p=m_impl->footers.at(static_cast<int>(t)); if(!p)p=std::make_unique<Content>(); return *p; }
Section& Section::linkHeaderToPrevious(HeaderFooterType t) { auto& p=m_impl->headers.at(static_cast<int>(t)); if(p)m_impl->retired.push_back(std::move(p)); return *this; }
Section& Section::linkFooterToPrevious(HeaderFooterType t) { auto& p=m_impl->footers.at(static_cast<int>(t)); if(p)m_impl->retired.push_back(std::move(p)); return *this; }
Section& Section::clearHeader(HeaderFooterType t) { linkHeaderToPrevious(t); header(t); return *this; }
Section& Section::clearFooter(HeaderFooterType t) { linkFooterToPrevious(t); footer(t); return *this; }
const Content* Section::headerContent(HeaderFooterType t) const { return m_impl->headers.at(static_cast<int>(t)).get(); }
const Content* Section::footerContent(HeaderFooterType t) const { return m_impl->footers.at(static_cast<int>(t)).get(); }
void Section::inheritLayout(const Section& p) { m_impl->page=p.m_impl->page; m_impl->columns=p.m_impl->columns; m_impl->space=p.m_impl->space; m_impl->separator=p.m_impl->separator; m_impl->widths=p.m_impl->widths; m_impl->pageFormat=p.m_impl->pageFormat; m_impl->title=p.m_impl->title; m_impl->footnotes=p.m_impl->footnotes; m_impl->endnotes=p.m_impl->endnotes; m_impl->grid=p.m_impl->grid; m_impl->gridPitch=p.m_impl->gridPitch; }
int Section::contentWidth() const {
    const auto& p=page(); int w=p.customWidth?p.customWidth:pageWidthDxa(p.size,p.orientation);
    return w-Length::cm(p.margins.left+p.margins.right).dxa()-p.gutter.dxa();
}
int Section::columnWidth() const {
    if(m_impl->widths.empty())return (contentWidth()-(m_impl->columns-1)*m_impl->space.dxa())/m_impl->columns;
    int width=contentWidth(); for(const auto& c:m_impl->widths)width=std::min(width,c.width.dxa()); return width;
}
std::string Section::propertiesXml() const {
    const auto& p=page(); int w=p.customWidth?p.customWidth:pageWidthDxa(p.size,p.orientation),h=p.customHeight?p.customHeight:pageHeightDxa(p.size,p.orientation);
    if(w<=0||h<=0||w>31680||h>31680||contentWidth()<=0||Length::cm(p.margins.top+p.margins.bottom).dxa()>=h) throw std::invalid_argument("Invalid page geometry");
    if(p.margins.top<0||p.margins.bottom<0||p.margins.left<0||p.margins.right<0||p.headerDistance.dxa()<0||p.footerDistance.dxa()<0||p.gutter.dxa()<0) throw std::invalid_argument("Negative page margins");
    std::string x;
    auto note=[&](const NoteOptions& o,bool end) {
        const char* restart[]={"continuous","eachSect","eachPage"}; std::string tag=end?"endnotePr":"footnotePr";
        x+="<w:"+tag+"><w:pos w:val=\""+std::string(end?(o.endnotePosition==EndnotePosition::DocumentEnd?"docEnd":"sectEnd"):(o.footnotePosition==FootnotePosition::PageBottom?"pageBottom":"beneathText"))+"\"/><w:numFmt w:val=\""+numberFormatName(o.format)+"\"/><w:numStart w:val=\""+std::to_string(o.start)+"\"/><w:numRestart w:val=\""+restart[static_cast<int>(o.restart)]+"\"/></w:"+tag+">";
    };
    if(m_impl->footnotes)note(*m_impl->footnotes,false); if(m_impl->endnotes)note(*m_impl->endnotes,true);
    x+="<w:type w:val=\""+sectionBreakTypeToString(m_impl->type)+"\"/><w:pgSz w:w=\""+std::to_string(w)+"\" w:h=\""+std::to_string(h)+"\""+(p.orientation==Orientation::Landscape?" w:orient=\"landscape\"":"")+"/>";
    x+="<w:pgMar w:top=\""+std::to_string(Length::cm(p.margins.top).dxa())+"\" w:right=\""+std::to_string(Length::cm(p.margins.right).dxa())+"\" w:bottom=\""+std::to_string(Length::cm(p.margins.bottom).dxa())+"\" w:left=\""+std::to_string(Length::cm(p.margins.left).dxa())+"\" w:header=\""+std::to_string(p.headerDistance.dxa())+"\" w:footer=\""+std::to_string(p.footerDistance.dxa())+"\" w:gutter=\""+std::to_string(p.gutter.dxa())+"\"/>";
    x+="<w:pgNumType w:fmt=\""+numberFormatName(m_impl->pageFormat)+"\""+(m_impl->start?" w:start=\""+std::to_string(m_impl->start)+"\"":"")+"/>";
    if(contentWidth()-(m_impl->columns-1)*m_impl->space.dxa()<=0) throw std::invalid_argument("Column spacing exceeds page width");
    x+="<w:cols w:num=\""+std::to_string(m_impl->columns)+"\" w:space=\""+std::to_string(m_impl->space.dxa())+"\" w:sep=\""+(m_impl->separator?"1":"0")+"\" w:equalWidth=\""+(m_impl->widths.empty()?"1":"0")+"\">";
    int sum=0;
    for(size_t i=0;i<m_impl->widths.size();++i) { const auto& c=m_impl->widths[i]; sum+=c.width.dxa()+(i+1<m_impl->widths.size()?c.space.dxa():0); x+="<w:col w:w=\""+std::to_string(c.width.dxa())+"\" w:space=\""+std::to_string(c.space.dxa())+"\"/>"; }
    if(sum>contentWidth()) throw std::invalid_argument("Columns exceed page width");
    x+="</w:cols>"; if(m_impl->title)x+="<w:titlePg/>";
    // w:docGrid follows w:titlePg in CT_SectPr order.
    if(m_impl->grid!=DocGridType::Default)x+="<w:docGrid w:type=\""+std::string(docGridTypeToString(m_impl->grid))+"\" w:linePitch=\""+std::to_string(m_impl->gridPitch)+"\"/>";
    return x;
}
}
