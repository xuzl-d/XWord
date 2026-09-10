#include "xword/Paragraph.hpp"
#include "xword/Equation.hpp"
#include "internal/ZipWriter.hpp"
#include "internal/utf.hpp"
#include <vector>

namespace xword {
using internal::xmlEscape;
struct Paragraph::Impl {
    struct Run { std::string kind, text, extra; RunStyle style; Image* image = nullptr; };
    std::vector<Run> runs;
    std::vector<std::unique_ptr<Image>> images;
    ParagraphStyle format;
    RunStyle defaults;
    std::string styleId;
    TargetId bookmark;
    int numId = -1, level = 0;
};
Paragraph::Paragraph() : m_impl(std::make_unique<Impl>()) {}
Paragraph::~Paragraph() = default;
Paragraph::Paragraph(Paragraph&&) noexcept = default;
Paragraph& Paragraph::operator=(Paragraph&&) noexcept = default;
Paragraph& Paragraph::addRun(const std::string& s) { return addRun(s,RunStyle()); }
Paragraph& Paragraph::addRun(const std::string& s,const RunStyle& style) { m_impl->runs.push_back({"text",s,"",style}); return *this; }
Paragraph& Paragraph::addRun(const std::wstring& s) { return addRun(internal::wstring_to_utf8(s)); }
Paragraph& Paragraph::addRun(const std::wstring& s,const RunStyle& style) { return addRun(internal::wstring_to_utf8(s),style); }
Paragraph& Paragraph::addEquation(const std::string& s) { m_impl->runs.push_back({"math",s}); return *this; }
Paragraph& Paragraph::addEquation(const std::wstring& s) { return addEquation(internal::wstring_to_utf8(s)); }
Paragraph& Paragraph::addField(const std::string& s,const std::string& cached) { m_impl->runs.push_back({"field",s,cached}); return *this; }
Paragraph& Paragraph::addPageNumber() { return addField("PAGE"); }
Paragraph& Paragraph::addPageCount() { return addField("NUMPAGES"); }
Paragraph& Paragraph::addSectionPageCount() { return addField("SECTIONPAGES"); }
Paragraph& Paragraph::addFootnoteRef(int id) { m_impl->runs.push_back({"footnote",std::to_string(id)}); return *this; }
Paragraph& Paragraph::addEndnoteRef(int id) { m_impl->runs.push_back({"endnote",std::to_string(id)}); return *this; }
Paragraph& Paragraph::addBreak(BreakType t) { const char* names[]={"textWrapping","page","column"}; m_impl->runs.push_back({"break",names[static_cast<int>(t)]}); return *this; }
Paragraph& Paragraph::addTab() { m_impl->runs.push_back({"tab",""}); return *this; }
Paragraph& Paragraph::addHyperlink(const std::string& text,const std::string& url,const RunStyle& s) { m_impl->runs.push_back({"link",text,url,s}); return *this; }
Paragraph& Paragraph::startBookmark(const TargetId& t) { m_impl->runs.push_back({"bookmarkStart",t.name,std::to_string(static_cast<int>(t.kind))}); return *this; }
Paragraph& Paragraph::endBookmark(const TargetId& t) { m_impl->runs.push_back({"bookmarkEnd",t.name}); return *this; }
Paragraph& Paragraph::setBookmark(const TargetId& t) { m_impl->bookmark=t; return *this; }
Paragraph& Paragraph::addReference(const TargetId& t,ReferenceKind k,bool link) {
    m_impl->runs.push_back({"ref",t.name,std::to_string(static_cast<int>(t.kind))+":"+std::to_string(static_cast<int>(k))+":"+(link?"1":"0")}); return *this;
}
Paragraph& Paragraph::addCitation(const std::string& tag,const std::string& pages) { m_impl->runs.push_back({"citation",tag,pages}); return *this; }
Paragraph& Paragraph::startComment(int id) { m_impl->runs.push_back({"commentStart",std::to_string(id)}); return *this; }
Paragraph& Paragraph::endComment(int id) { m_impl->runs.push_back({"commentEnd",std::to_string(id)}); return *this; }
Image& Paragraph::addImage(const std::string& path) {
    auto i=std::make_unique<Image>(path); auto p=i.get(); m_impl->images.push_back(std::move(i));
    m_impl->runs.push_back({"image","","",RunStyle(),p}); return *p;
}
Paragraph& Paragraph::setStyle(const RunStyle& s) { m_impl->defaults=s; return *this; }
RunStyle& Paragraph::getStyle() { return m_impl->defaults; }
Paragraph& Paragraph::setStyleId(const std::string& id) { m_impl->styleId=id; return *this; }
Paragraph& Paragraph::setParagraphStyle(const ParagraphStyle& s) { m_impl->format=s; m_impl->defaults=s.run; return *this; }
Paragraph& Paragraph::setAlignment(Alignment a) { m_impl->format.alignment=a; return *this; }
Paragraph& Paragraph::setFirstLineIndent(int n) { m_impl->format.firstLine=n<0 ? std::optional<Length>() : Length::twips(n); m_impl->format.hanging.reset(); m_impl->format.firstLineChars.reset(); return *this; }
Paragraph& Paragraph::setFirstLineIndentChars(double n,int) { if(n<0) throw std::invalid_argument("Negative indent"); m_impl->format.firstLineChars=n; m_impl->format.firstLine.reset(); m_impl->format.hanging.reset(); return *this; }
Paragraph& Paragraph::setSpacingBefore(int n) { m_impl->format.before=n<0 ? std::optional<Length>() : Length::twips(n); return *this; }
Paragraph& Paragraph::setSpacingAfter(int n) { m_impl->format.after=n<0 ? std::optional<Length>() : Length::twips(n); return *this; }
Paragraph& Paragraph::setPageBreakBefore(bool on) { m_impl->format.pageBreakBefore=on?Toggle::On:Toggle::Off; return *this; }
Paragraph& Paragraph::setKeepWithNext(bool on) { m_impl->format.keepNext=on?Toggle::On:Toggle::Off; return *this; }
Paragraph& Paragraph::setKeepTogether(bool on) { m_impl->format.keepLines=on?Toggle::On:Toggle::Off; return *this; }
Paragraph& Paragraph::setWidowControl(bool on) { m_impl->format.widowControl=on?Toggle::On:Toggle::Off; return *this; }
Paragraph& Paragraph::setLeftIndent(Length n) { m_impl->format.leftIndent=n; return *this; }
Paragraph& Paragraph::setRightIndent(Length n) { m_impl->format.rightIndent=n; return *this; }
Paragraph& Paragraph::setHangingIndent(Length n) { m_impl->format.hanging=n; m_impl->format.firstLine.reset(); m_impl->format.firstLineChars.reset(); return *this; }
Paragraph& Paragraph::setLineSpacing(double n,LineRule r) { if(!std::isfinite(n)||n<=0) throw std::invalid_argument("Invalid line spacing"); m_impl->format.lineSpacing=n; m_impl->format.lineRule=r; return *this; }
Paragraph& Paragraph::addTabStop(const TabStop& t) { m_impl->format.tabs.push_back(t); return *this; }
Paragraph& Paragraph::setBorder(const Border& b) { m_impl->format.border=b; return *this; }
Paragraph& Paragraph::setShading(const std::string& c) { m_impl->format.shading=c; return *this; }
Paragraph& Paragraph::setNumbering(int id,int level) { if(id<0||level<0||level>8) throw std::invalid_argument("Invalid numbering"); m_impl->numId=id; m_impl->level=level; return *this; }
std::string Paragraph::toXml() const {
    std::string x="<w:p><w:pPr>";
    if(!m_impl->styleId.empty()) x+="<w:pStyle w:val=\""+xmlEscape(m_impl->styleId)+"\"/>";
    // numPr is inserted in schema order by the package serializer.
    if(m_impl->numId>=0) x+="<w:numPr><w:ilvl w:val=\""+std::to_string(m_impl->level)+"\"/><w:numId w:val=\""+std::to_string(m_impl->numId)+"\"/></w:numPr>";
    x+=m_impl->format.toXml()+"</w:pPr>";
    auto mark=[&](bool start,const std::string& name,const std::string& kind="0") { return "<xw:"+std::string(start?"bookmarkStart":"bookmarkEnd")+" name=\""+xmlEscape(name)+"\" kind=\""+kind+"\"/>"; };
    if(!m_impl->bookmark.name.empty()) x+=mark(true,m_impl->bookmark.name,std::to_string(static_cast<int>(m_impl->bookmark.kind)));
    auto textRun=[&](const std::string& s,const RunStyle& style) { return "<w:r>"+style.toXml(&m_impl->defaults)+"<w:t xml:space=\"preserve\">"+xmlEscape(s)+"</w:t></w:r>"; };
    for(const auto& r:m_impl->runs) {
        if(r.kind=="text") {
            size_t start=0;
            for(size_t i=0;i<r.text.size();++i) if(r.text[i]=='\n'||r.text[i]=='\t'||r.text[i]=='\r') {
                if(i>start) x+=textRun(r.text.substr(start,i-start),r.style);
                if(r.text[i]=='\t') x+="<w:r><w:tab/></w:r>";
                else { x+="<w:r><w:br/></w:r>"; if(r.text[i]=='\r'&&i+1<r.text.size()&&r.text[i+1]=='\n') ++i; }
                start=i+1;
            }
            if(start<r.text.size()||r.text.empty()) x+=textRun(r.text.substr(start),r.style);
        } else if(r.kind=="math") { Equation eq(r.text); eq.setStyle(m_impl->defaults); x+=eq.toXml(); }
        else if(r.kind=="field") x+="<w:r><w:fldChar w:fldCharType=\"begin\" w:dirty=\"true\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> "+xmlEscape(r.text)+" </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>"+textRun(r.extra,RunStyle())+"<w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
        else if(r.kind=="footnote"||r.kind=="endnote") x+="<w:r><w:rPr><w:rStyle w:val=\""+std::string(r.kind=="footnote"?"FootnoteReference":"EndnoteReference")+"\"/></w:rPr><w:"+r.kind+"Reference w:id=\""+r.text+"\"/></w:r>";
        else if(r.kind=="break") x+="<w:r><w:br w:type=\""+r.text+"\"/></w:r>";
        else if(r.kind=="tab") x+="<w:r><w:tab/></w:r>";
        else if(r.kind=="link") x+="<w:hyperlink xw:url=\""+xmlEscape(r.extra)+"\">"+textRun(r.text,r.style)+"</w:hyperlink>";
        else if(r.kind=="bookmarkStart") x+=mark(true,r.text,r.extra);
        else if(r.kind=="bookmarkEnd") x+=mark(false,r.text);
        else if(r.kind=="ref"||r.kind=="citation") x+="<xw:"+r.kind+" target=\""+xmlEscape(r.text)+"\" options=\""+xmlEscape(r.extra)+"\"/>";
        else if(r.kind=="commentStart") x+="<w:commentRangeStart w:id=\""+r.text+"\"/>";
        else if(r.kind=="commentEnd") x+="<w:commentRangeEnd w:id=\""+r.text+"\"/><w:r><w:commentReference w:id=\""+r.text+"\"/></w:r>";
        else if(r.kind=="image") x+=r.image->toXml(true);
    }
    if(!m_impl->bookmark.name.empty()) x+=mark(false,m_impl->bookmark.name);
    return x+"</w:p>";
}
}
