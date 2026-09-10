#include "xword/Document.hpp"
#include "internal/Package.hpp"
#include "internal/utf.hpp"
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <map>
#include <regex>
#include <functional>

namespace xword {
using namespace internal;
struct Document::Impl {
    struct SectionEntry { std::unique_ptr<Section> settings; std::unique_ptr<Content> body; };
    std::vector<SectionEntry> sections;
    std::map<int,std::unique_ptr<Note>> footnotes,endnotes;
    struct Comment { std::string text,author,date; };
    std::vector<Comment> comments;
    std::map<std::string,BibliographySource> sources;
    std::map<std::string,ParagraphStyle> paragraphStyles;
    std::map<std::string,std::pair<RunStyle,std::string>> characterStyles;
    std::map<std::string,TableStyleDefinition> tableStyles;
    DocumentProperties properties;
    std::map<std::string,CustomProperty> custom;
    HeadingStyle headingStyles[9];
    RunStyle bodyStyle,tableStyle,equationStyle;
    std::string eastAsia,ascii,hAnsi,bibliographyStyle="IEEE";
    double bodySize=11,bodySpacing=1.15;
    int indent=480;
    bool headingNumbering=false,evenOdd=false,imageNumbering=false,tableNumbering=false,isTemplate=false;
    HeadingNumFormat headingFormat=HeadingNumFormat::Decimal;
    CaptionNumStyle imageNumStyle=CaptionNumStyle::Sequential,tableNumStyle=CaptionNumStyle::Sequential;
    std::string imagePrefix=u8"图",tablePrefix=u8"表";
    std::map<std::string,std::string> templateParts,vars;
    std::map<std::string,std::unique_ptr<Content>> blocks;
    void requireGeneration() const { if(isTemplate)throw std::invalid_argument("This document-level operation requires generation mode; template mode supports set* content replacement"); }
    Content& body() { if(isTemplate)throw std::invalid_argument("Use set* content factories in template mode"); return *sections.back().body; }
    Content& block(const std::string& key) { auto& p=blocks[key]; if(!p)p=std::make_unique<Content>(); return *p; }
};
Document::Document():m_impl(std::make_unique<Impl>()) { addSection(); }
Document::~Document()=default;
Section& Document::currentSection() { m_impl->requireGeneration(); return *m_impl->sections.back().settings; }
Section& Document::addSection(SectionBreakType t) {
    if(!m_impl->sections.empty())m_impl->requireGeneration();
    auto s=std::make_unique<Section>(t); if(!m_impl->sections.empty())s->inheritLayout(currentSection());
    auto p=s.get(); m_impl->sections.push_back({std::move(s),std::make_unique<Content>()}); return *p;
}
Document& Document::setPage(const Page& p) { currentSection().setPage(p); return *this; }
Document& Document::addSectionBreak(SectionBreakType t) { addSection(t); return *this; }
Document& Document::addPageBreak() { addParagraph().setFirstLineIndent(0).addBreak(BreakType::Page); return *this; }
Document& Document::enableTitlePage() { currentSection().setTitlePage(); return *this; }
Document& Document::setEvenAndOddHeaders(bool on) { m_impl->requireGeneration(); m_impl->evenOdd=on; return *this; }
Document& Document::setDefaultParagraphIndent(double chars,int size) { m_impl->requireGeneration(); if(!std::isfinite(chars)||chars<0||size<=0)throw std::invalid_argument("Invalid paragraph indent"); m_impl->indent=static_cast<int>(chars*size*20); return *this; }
Document& Document::setBodyFont(const std::string& ea,const std::string& ascii,const std::string& hAnsi) { m_impl->requireGeneration(); m_impl->eastAsia=ea; m_impl->ascii=ascii.empty()?ea:ascii; m_impl->hAnsi=hAnsi.empty()?m_impl->ascii:hAnsi; return *this; }
Document& Document::setBodyFontSize(double n) { m_impl->requireGeneration(); if(!std::isfinite(n)||n<=0||n>1638)throw std::invalid_argument("Invalid font size"); m_impl->bodySize=n; return *this; }
Document& Document::setBodyLineSpacing(double n) { m_impl->requireGeneration(); if(!std::isfinite(n)||n<=0)throw std::invalid_argument("Invalid line spacing"); m_impl->bodySpacing=n; return *this; }
Document& Document::setBodyRunStyle(const RunStyle& s) { m_impl->requireGeneration(); m_impl->bodyStyle=s; return *this; }
Document& Document::setTableRunStyle(const RunStyle& s) { m_impl->requireGeneration(); m_impl->tableStyle=s; return *this; }
Document& Document::setDisplayEquationStyle(const RunStyle& s) { m_impl->requireGeneration(); m_impl->equationStyle=s; return *this; }
Paragraph& Document::addParagraph(const std::string& s) { return m_impl->body().addParagraph(s).setStyle(m_impl->bodyStyle).setFirstLineIndent(m_impl->indent); }
Paragraph& Document::addParagraph(const std::wstring& s) { return addParagraph(wstring_to_utf8(s)); }
Paragraph& Document::addHeadingParagraph(const std::string& s,int level,bool numbered) {
    if(level<1||level>9)throw std::invalid_argument("Heading level must be 1..9");
    auto& p=m_impl->body().addParagraph(s).setStyleId("Heading"+std::to_string(level)).setFirstLineIndent(0);
    if(numbered&&m_impl->headingNumbering)p.setNumbering(1,level-1); return p;
}
Document& Document::addHeading(const std::string& s,int l) { addHeadingParagraph(s,l); return *this; }
Document& Document::addHeading(const std::wstring& s,int l) { return addHeading(wstring_to_utf8(s),l); }
Document& Document::addHeadingNoNum(const std::string& s,int l) { addHeadingParagraph(s,l,false); return *this; }
Document& Document::addHeadingNoNum(const std::wstring& s,int l) { return addHeadingNoNum(wstring_to_utf8(s),l); }
Document& Document::setHeadingStyle(int l,const HeadingStyle& s) { m_impl->requireGeneration(); if(l<1||l>9)throw std::invalid_argument("Heading level must be 1..9"); m_impl->headingStyles[l-1]=s; return *this; }
Document& Document::enableHeadingNumbering() { m_impl->requireGeneration(); m_impl->headingNumbering=true; return *this; }
Document& Document::disableHeadingNumbering() { m_impl->requireGeneration(); m_impl->headingNumbering=false; return *this; }
Document& Document::setHeadingNumFormat(HeadingNumFormat f) { m_impl->requireGeneration(); m_impl->headingFormat=f; return *this; }
Document& Document::addTOC(const std::string& levels,const std::string& title) {
    if(!std::regex_match(levels,std::regex("[1-9]-[1-9]"))||levels[0]>levels[2])throw std::invalid_argument("TOC levels must be an ascending range in 1..9");
    if(!title.empty())addHeadingNoNum(title,1); addParagraph().setFirstLineIndent(0).addField("TOC \\o \""+levels+"\" \\h \\z \\u"); return *this;
}
Document& Document::addTOC(const std::wstring& l,const std::wstring& t) { return addTOC(wstring_to_utf8(l),wstring_to_utf8(t)); }
Document& Document::addFigureTOC(const std::string& t) { if(!t.empty())addHeadingNoNum(t,1); addParagraph().addField("TOC \\h \\z \\c \"Figure\""); return *this; }
Document& Document::addTableTOC(const std::string& t) { if(!t.empty())addHeadingNoNum(t,1); addParagraph().addField("TOC \\h \\z \\c \"Table\""); return *this; }
Image& Document::addImage(const std::string& s) { return m_impl->body().addImage(s); }
Image& Document::addImage(const std::filesystem::path& s) { return addImage(s.u8string()); }
Image& Document::addImage(const std::wstring& s) { return addImage(std::filesystem::path(s)); }
Table& Document::addTable(int r,int c) { return m_impl->body().addTable(r,c).setStyle(m_impl->tableStyle); }
BulletList& Document::addBulletList() { return m_impl->body().addBulletList(); }
BulletList& Document::addOrderedList() { return m_impl->body().addOrderedList(); }
Equation& Document::addEquation(const std::string& s) { return m_impl->body().addMath(s,EquationMode::Inline); }
Equation& Document::addDisplayEquation(const std::string& s) { return m_impl->body().addMath(s,EquationMode::Display).setStyle(m_impl->equationStyle); }
Document& Document::enableImageNumbering(const std::string& p,CaptionNumStyle s) { m_impl->imageNumbering=true; m_impl->imagePrefix=p; m_impl->imageNumStyle=s; return *this; }
Document& Document::disableImageNumbering() { m_impl->imageNumbering=false; return *this; }
Document& Document::enableTableNumbering(const std::string& p,CaptionNumStyle s) { m_impl->tableNumbering=true; m_impl->tablePrefix=p; m_impl->tableNumStyle=s; return *this; }
Document& Document::disableTableNumbering() { m_impl->tableNumbering=false; return *this; }
Paragraph& Document::setHeader() { currentSection().clearHeader(); return currentSection().header().addParagraph(); }
Paragraph& Document::setFooter() { currentSection().clearFooter(); return currentSection().footer().addParagraph(); }
Document& Document::setHeader(const std::string& s) { setHeader().addRun(s).setAlignment(Alignment::Center); return *this; }
Document& Document::setFooter(const std::string& s) { setFooter().addRun(s).setAlignment(Alignment::Center); return *this; }
void Document::clearHeader() { currentSection().clearHeader(); } void Document::clearFooter() { currentSection().clearFooter(); }
Note& Document::addFootnote() { m_impl->requireGeneration(); int id=static_cast<int>(m_impl->footnotes.size())+1; auto p=std::make_unique<Note>(id); auto result=p.get(); m_impl->footnotes[id]=std::move(p); return *result; }
Note& Document::addEndnote() { m_impl->requireGeneration(); int id=static_cast<int>(m_impl->endnotes.size())+1; auto p=std::make_unique<Note>(id); auto result=p.get(); m_impl->endnotes[id]=std::move(p); return *result; }
int Document::addFootnote(const std::string& text) { m_impl->requireGeneration(); auto& n=addFootnote(); n.addParagraph(text); return n.id(); }
int Document::addEndnote(const std::string& text) { m_impl->requireGeneration(); auto& n=addEndnote(); n.addParagraph(text); return n.id(); }
Note& Document::footnote(int id) { return *m_impl->footnotes.at(id); } Note& Document::endnote(int id) { return *m_impl->endnotes.at(id); }
int Document::addComment(const std::string& text,const std::string& author,const std::string& date) { m_impl->requireGeneration();
    std::string timestamp=date;
    if(timestamp.empty()) { std::time_t now=std::time(nullptr); std::tm utc{};
#ifdef _WIN32
        gmtime_s(&utc,&now);
#else
        gmtime_r(&now,&utc);
#endif
        std::ostringstream s; s<<std::put_time(&utc,"%Y-%m-%dT%H:%M:%SZ"); timestamp=s.str();
    }
    if(!std::regex_match(timestamp,std::regex("[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}Z")))throw std::invalid_argument("Comment date must be UTC ISO 8601");
    int id=static_cast<int>(m_impl->comments.size()); m_impl->comments.push_back({text,author,timestamp}); return id;
}
Document& Document::addSource(const BibliographySource& source) { m_impl->requireGeneration();
    if(source.tag.empty()||source.title.empty()||source.tag.find_first_of("\"\\\r\n")!=std::string::npos)throw std::invalid_argument("Source requires a tag and title");
    if(m_impl->sources.count(source.tag))throw std::invalid_argument("Duplicate source tag: "+source.tag);
    m_impl->sources[source.tag]=source; return *this;
}
Document& Document::setBibliographyStyle(const std::string& s) { m_impl->requireGeneration(); if(!std::regex_match(s,std::regex("[A-Za-z0-9_-]+")))throw std::invalid_argument("Use a Word bibliography style name"); m_impl->bibliographyStyle=s; return *this; }
Document& Document::addBibliography(const std::string& title) { if(!title.empty())addHeadingNoNum(title,1); addParagraph().addField("BIBLIOGRAPHY"); return *this; }
Document& Document::setProperties(const DocumentProperties& p) { m_impl->requireGeneration(); m_impl->properties=p; return *this; }
Document& Document::setCustomProperty(const std::string& n,const CustomProperty& p) { m_impl->requireGeneration(); if(n.empty())throw std::invalid_argument("Empty custom property name"); m_impl->custom[n]=p; return *this; }
Document& Document::registerParagraphStyle(const std::string& id,const ParagraphStyle& s) { if(id.empty())throw std::invalid_argument("Empty style ID"); m_impl->paragraphStyles[id]=s; return *this; }
Document& Document::registerCharacterStyle(const std::string& id,const RunStyle& s,const std::string& base) { if(id.empty())throw std::invalid_argument("Empty style ID"); m_impl->characterStyles[id]={s,base}; return *this; }
Document& Document::registerTableStyle(const std::string& id,const TableStyleDefinition& s) { if(id.empty())throw std::invalid_argument("Empty style ID"); m_impl->tableStyles[id]=s; return *this; }

std::string Document::buildStylesXml() {
    pugi::xml_document d; parseXml(d,documentXml("w:styles","")); auto root=d.document_element();
    auto defaults=root.append_child("w:docDefaults"); RunStyle normal; normal.fontSize(m_impl->bodySize).font(m_impl->ascii).eastAsiaFont(m_impl->eastAsia);
    appendXml(defaults.append_child("w:rPrDefault"),normal.toXml());
    auto fonts=defaults.child("w:rPrDefault").child("w:rPr").child("w:rFonts");
    if(fonts&&!m_impl->hAnsi.empty())fonts.attribute("w:hAnsi")=m_impl->hAnsi.c_str();
    ParagraphStyle ps; ps.lineSpacing=m_impl->bodySpacing; ps.after=Length::pt(8); appendXml(defaults.append_child("w:pPrDefault").append_child("w:pPr"),ps.toXml());
    auto style=[&](const std::string& id,const char* type,const std::string& base) {
        for(auto old=root.first_child();old;) { auto next=old.next_sibling(); if(std::string(old.name())=="w:style"&&id==old.attribute("w:styleId").value())root.remove_child(old); old=next; }
        auto n=root.append_child("w:style"); n.append_attribute("w:type")=type; n.append_attribute("w:styleId")=id.c_str(); child(n,"w:name","w:val",id); if(!base.empty())child(n,"w:basedOn","w:val",base); return n;
    };
    auto norm=style("Normal","paragraph",""); norm.append_attribute("w:default")="1";
    for(int level=1;level<=9;++level) {
        const auto& h=m_impl->headingStyles[level-1]; auto n=style("Heading"+std::to_string(level),"paragraph","Normal"); child(n,"w:next","w:val","Normal"); n.append_child("w:qFormat");
        ParagraphStyle p; p.keepNext=Toggle::On; p.keepLines=Toggle::On; p.before=Length::pt(h.spaceBefore>=0?h.spaceBefore:12); p.after=Length::pt(h.spaceAfter>=0?h.spaceAfter:6); p.lineSpacing=h.lineSpacing; if(h.hasAlignment)p.alignment=h.alignment;
        auto pr=n.append_child("w:pPr"); appendXml(pr,p.toXml()); child(pr,"w:outlineLvl","w:val",std::to_string(level-1));
        RunStyle r; r.bold(h.bold).italic(h.italic).font(h.font).eastAsiaFont(h.font).fontSize(h.fontSize>0?h.fontSize:std::max(11,28-level*2)).color(h.color); appendXml(n,r.toXml());
    }
    for(auto id:{"Caption","ListParagraph","FootnoteText","EndnoteText","CommentText"})style(id,"paragraph","Normal");
    for(auto id:{"FootnoteReference","EndnoteReference"})appendXml(style(id,"character",""),RunStyle().superscript().toXml());
    appendXml(style("Hyperlink","character",""),RunStyle().color("0563C1").underline().toXml());
    for(const auto& e:m_impl->paragraphStyles) { auto n=style(e.first,"paragraph",e.second.basedOn); if(!e.second.next.empty())child(n,"w:next","w:val",e.second.next); appendXml(n.append_child("w:pPr"),e.second.toXml()); appendXml(n,e.second.run.toXml()); }
    for(const auto& e:m_impl->characterStyles)appendXml(style(e.first,"character",e.second.second),e.second.first.toXml());
    for(const auto& e:m_impl->tableStyles) {
        auto n=style(e.first,"table",e.second.basedOn); appendXml(n,e.second.run.toXml());
        Table table(1,1); if(e.second.border)table.setBorder(*e.second.border); table.setShading(e.second.shading).setCellMargins(e.second.cellMargin,e.second.cellMargin,e.second.cellMargin,e.second.cellMargin);
        pugi::xml_document t; parseXml(t,documentXml("xw:root",table.toXml())); auto tp=t.document_element().child("w:tbl").child("w:tblPr"); tp.remove_child("w:tblW"); tp.remove_child("w:jc"); tp.remove_child("w:tblLayout"); n.append_copy(tp);
    }
    // Style inheritance must reference existing styles without cycles.
    std::map<std::string,std::string> bases;
    for(auto n:root.children("w:style"))bases[n.attribute("w:styleId").value()]=n.child("w:basedOn").attribute("w:val").value();
    if(m_impl->isTemplate&&m_impl->templateParts.count("word/styles.xml")) {
        pugi::xml_document existing; parseXml(existing,m_impl->templateParts.at("word/styles.xml")); canonicalizeWordPrefixes(existing.document_element());
        for(auto n:existing.document_element().children("w:style")) { std::string id=n.attribute("w:styleId").value(); if(!bases.count(id))bases[id]=n.child("w:basedOn").attribute("w:val").value(); }
    }
    for(const auto& b:bases) { std::set<std::string> chain; auto s=b.first; while(!s.empty()) { if(!bases.count(s)||!chain.insert(s).second)throw std::invalid_argument("Invalid style inheritance at "+s); s=bases.at(s); } }
    return xmlString(d);
}
std::string Document::buildNumberingXml() {
    pugi::xml_document d; parseXml(d,documentXml("w:numbering","")); auto r=d.document_element(); auto a=r.append_child("w:abstractNum"); a.append_attribute("w:abstractNumId")=1; child(a,"w:multiLevelType","w:val","multilevel");
    for(int i=0;i<9;++i) {
        auto l=a.append_child("w:lvl"); l.append_attribute("w:ilvl")=i; child(l,"w:start","w:val","1"); child(l,"w:numFmt","w:val","decimal");
        std::string text; for(int j=0;j<=i;++j) {if(j)text+="."; text+="%"+std::to_string(j+1);}
        if(i==0&&m_impl->headingFormat==HeadingNumFormat::Chapter)text=u8"第 %1 章";
        child(l,"w:lvlText","w:val",text); child(l,"w:lvlJc","w:val","left");
    }
    auto n=r.append_child("w:num"); n.append_attribute("w:numId")=1; child(n,"w:abstractNumId","w:val","1"); return xmlString(d);
}
namespace {
std::string notesXml(const std::map<int,std::unique_ptr<Note>>& notes,bool end) {
    std::string tag=end?"endnote":"footnote",content;
    for(int i=-1;i<=0;++i)content+="<w:"+tag+" w:type=\""+(i==-1?"separator":"continuationSeparator")+"\" w:id=\""+std::to_string(i)+"\"><w:p><w:r><w:"+(i==-1?"separator":"continuationSeparator")+"/></w:r></w:p></w:"+tag+">";
    for(const auto& entry:notes) {
        pugi::xml_document f; parseXml(f,documentXml("xw:root",entry.second->toXml())); auto p=f.document_element().first_child(); if(std::string(p.name())!="w:p")p=f.document_element().prepend_child("w:p");
        auto pr=p.child("w:pPr"); if(!pr)pr=p.prepend_child("w:pPr"); if(!pr.child("w:pStyle"))pr.prepend_child("w:pStyle").append_attribute("w:val")=end?"EndnoteText":"FootnoteText";
        auto run=p.insert_child_after("w:r",pr); appendXml(run,"<w:rPr><w:rStyle w:val=\""+std::string(end?"EndnoteReference":"FootnoteReference")+"\"/></w:rPr><w:"+tag+"Ref/>");
        content+="<w:"+tag+" w:id=\""+std::to_string(entry.first)+"\">"; for(auto n:f.document_element().children())content+=xmlString(n); content+="</w:"+tag+">";
    }
    return documentXml("w:"+tag+"s",content);
}
void addMetadata(Package& package,const DocumentProperties& props,const std::map<std::string,CustomProperty>& custom) {
    pugi::xml_document d; auto r=d.append_child("cp:coreProperties"); r.append_attribute("xmlns:cp")="http://schemas.openxmlformats.org/package/2006/metadata/core-properties"; r.append_attribute("xmlns:dc")="http://purl.org/dc/elements/1.1/";
    auto text=[&](const char* name,const std::string& s){if(!s.empty())r.append_child(name).text().set(s.c_str());};
    text("dc:title",props.title); text("dc:subject",props.subject); text("dc:creator",props.creator); text("cp:keywords",props.keywords); text("dc:description",props.description); text("cp:lastModifiedBy",props.lastModifiedBy); text("dc:language",props.language);
    package.addPart("docProps/core.xml",xmlString(d),"application/vnd.openxmlformats-package.core-properties+xml"); package.relate("","docProps/core.xml","http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties");
    if(custom.empty())return;
    pugi::xml_document c; auto cr=c.append_child("Properties"); cr.append_attribute("xmlns")="http://schemas.openxmlformats.org/officeDocument/2006/custom-properties"; cr.append_attribute("xmlns:vt")="http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes";
    int id=2; for(const auto& entry:custom) {
        const auto& v=entry.second;
        if(v.type==PropertyType::Number) { size_t used=0; double value=std::stod(v.value,&used); if(used!=v.value.size()||!std::isfinite(value))throw std::invalid_argument("Invalid numeric property"); }
        if(v.type==PropertyType::Boolean&&v.value!="true"&&v.value!="false")throw std::invalid_argument("Boolean property must be true or false");
        if(v.type==PropertyType::Date&&!std::regex_match(v.value,std::regex("[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}Z")))throw std::invalid_argument("Date property must be UTC ISO 8601");
        auto p=cr.append_child("property"); p.append_attribute("fmtid")="{D5CDD505-2E9C-101B-9397-08002B2CF9AE}"; p.append_attribute("pid")=id++; p.append_attribute("name")=entry.first.c_str(); const char* tags[]={"vt:lpwstr","vt:r8","vt:bool","vt:filetime"}; p.append_child(tags[static_cast<int>(v.type)]).text().set(v.value.c_str());
    }
    package.addPart("docProps/custom.xml",xmlString(c),"application/vnd.openxmlformats-officedocument.custom-properties+xml"); package.relate("","docProps/custom.xml",std::string(relNs())+"custom-properties");
}
}
bool Document::save(const std::string& path) { return saveDetailed(path).success; }
SaveResult Document::saveDetailed(const std::string& path,const SaveOptions& options) {
    Package package; package.options=options;
    SaveResult result;
    try {
        package.imageNumbering=m_impl->imageNumbering; package.tableNumbering=m_impl->tableNumbering; package.imagePrefix=m_impl->imagePrefix; package.tablePrefix=m_impl->tablePrefix;
        package.imageByChapter=m_impl->imageNumStyle==CaptionNumStyle::ByChapter; package.tableByChapter=m_impl->tableNumStyle==CaptionNumStyle::ByChapter;
        if(m_impl->isTemplate) {
            package.parts=m_impl->templateParts;
            for(auto& entry:package.parts)if(entry.first=="word/document.xml"||entry.first.find("word/header")==0||entry.first.find("word/footer")==0)if(entry.first.size()>4&&entry.first.substr(entry.first.size()-4)==".xml")entry.second=renderXml(entry.second);
            // Merge required built-in/new styles, retaining template definitions.
            pugi::xml_document styles,generated; parseXml(generated,buildStylesXml());
            if(package.parts.count("word/styles.xml"))parseXml(styles,package.parts.at("word/styles.xml")); else parseXml(styles,documentXml("w:styles",""));
            std::set<std::string> ids; for(auto n:styles.document_element().children("w:style"))ids.insert(n.attribute("w:styleId").value());
            for(auto n:generated.document_element().children("w:style")) {
                std::string id=n.attribute("w:styleId").value(); bool explicitStyle=m_impl->paragraphStyles.count(id)||m_impl->characterStyles.count(id)||m_impl->tableStyles.count(id);
                if(ids.count(id)&&explicitStyle)for(auto old=styles.document_element().first_child();old;) { auto next=old.next_sibling(); if(id==old.attribute("w:styleId").value())styles.document_element().remove_child(old); old=next; }
                if(!ids.count(id)||explicitStyle)styles.document_element().append_copy(n);
            }
            package.addPart("word/styles.xml",xmlString(styles),Package::wordType("styles"));
        } else {
            package.addPart("word/styles.xml",buildStylesXml(),Package::wordType("styles"));
            package.addPart("word/numbering.xml",buildNumberingXml(),Package::wordType("numbering"));
            std::string body;
            for(size_t i=0;i<m_impl->sections.size();++i) {
                const auto& entry=m_impl->sections[i]; const auto& sec=*entry.settings;
                std::string refs; const char* hfTypes[]={"default","first","even"};
                for(bool footer:{false,true})for(int type=0;type<3;++type) {
                    auto c=footer?sec.footerContent(static_cast<HeaderFooterType>(type)):sec.headerContent(static_cast<HeaderFooterType>(type)); if(!c)continue;
                    std::string tag=footer?"footer":"header",file=tag+std::to_string(i+1)+"_"+hfTypes[type]+".xml";
                    std::string contents=c->toXml(); if(contents.empty())contents="<w:p/>";
                    package.addPart("word/"+file,documentXml(footer?"w:ftr":"w:hdr","<xw:section width=\""+std::to_string(sec.contentWidth())+"\">"+contents+"</xw:section>"),Package::wordType(tag));
                    auto id=package.relate("word/document.xml",file,std::string(relNs())+tag);
                    refs+="<w:"+tag+"Reference w:type=\""+hfTypes[type]+"\" r:id=\""+id+"\"/>";
                }
                body+="<xw:section width=\""+std::to_string(sec.columnWidth())+"\">"+entry.body->toXml();
                std::string sect="<w:sectPr>"+refs+sec.propertiesXml()+"</w:sectPr>";
                if(i+1<m_impl->sections.size())body+="<w:p><w:pPr>"+sect+"</w:pPr></w:p>";
                else body+=sect;
                body+="</xw:section>";
            }
            package.addPart("word/document.xml",documentXml("w:document","<w:body>"+body+"</w:body>"),Package::wordType("document.main"));
            package.relate("","word/document.xml",std::string(relNs())+"officeDocument");
            addMetadata(package,m_impl->properties,m_impl->custom);
        }
        package.relate("word/document.xml","styles.xml",std::string(relNs())+"styles");
        for(bool end:{false,true}) {
            const auto& notes=end?m_impl->endnotes:m_impl->footnotes; if(notes.empty())continue;
            std::string name=end?"endnotes":"footnotes";
            package.addPart("word/"+name+".xml",notesXml(notes,end),Package::wordType(name)); package.relate("word/document.xml",name+".xml",std::string(relNs())+name);
        }
        if(!m_impl->comments.empty()) {
            std::string xml; for(size_t i=0;i<m_impl->comments.size();++i) { const auto& c=m_impl->comments[i]; Paragraph p; p.addRun(c.text); xml+="<w:comment w:id=\""+std::to_string(i)+"\" w:author=\""+xmlEscape(c.author)+"\" w:date=\""+xmlEscape(c.date)+"\">"+p.toXml()+"</w:comment>"; }
            package.addPart("word/comments.xml",documentXml("w:comments",xml),Package::wordType("comments")); package.relate("word/document.xml","comments.xml",std::string(relNs())+"comments");
        }
        pugi::xml_document settings;
        if(package.parts.count("word/settings.xml"))parseXml(settings,package.parts.at("word/settings.xml")); else parseXml(settings,documentXml("w:settings",""));
        auto sr=settings.document_element(); auto update=sr.child("w:updateFields");
        if(!update) {
            pugi::xml_node before;
            const std::set<std::string> following={"w:footnotePr","w:endnotePr","w:compat","w:docVars","w:rsids","m:mathPr","w:attachedSchema","w:themeFontLang","w:clrSchemeMapping","w:doNotIncludeSubdocsInStats","w:doNotAutoCompressPictures","w:forceUpgrade","w:captions","w:readModeInkLockDown","w:smartTagType","sl:schemaLibrary","w:shapeDefaults","w:decimalSymbol","w:listSeparator"};
            for(auto n:sr.children())if(following.count(n.name())){before=n;break;}
            update=before?sr.insert_child_before("w:updateFields",before):sr.append_child("w:updateFields"); update.append_attribute("w:val");
        }
        if(!update.attribute("w:val"))update.append_attribute("w:val"); update.attribute("w:val")="true";
        if(m_impl->evenOdd&&!sr.child("w:evenAndOddHeaders"))sr.prepend_child("w:evenAndOddHeaders");
        for(bool end:{false,true}) {
            if((end?m_impl->endnotes:m_impl->footnotes).empty())continue;
            const char* tag=end?"w:endnotePr":"w:footnotePr"; auto pr=sr.child(tag); if(!pr)pr=sr.append_child(tag);
            for(int id:{-1,0}) { auto n=pr.append_child(end?"w:endnote":"w:footnote"); n.append_attribute("w:id")=id; }
        }
        // Compatibility mode 15 = Word 2013 and newer; no layout engine dependency.
        if(!sr.child("w:compat")) { auto c=sr.append_child("w:compat").append_child("w:compatSetting"); c.append_attribute("w:name")="compatibilityMode"; c.append_attribute("w:uri")="http://schemas.microsoft.com/office/word"; c.append_attribute("w:val")="15"; }
        package.addPart("word/settings.xml",xmlString(settings),Package::wordType("settings")); package.relate("word/document.xml","settings.xml",std::string(relNs())+"settings");
        if(!m_impl->sources.empty()) {
            pugi::xml_document b; auto root=b.append_child("b:Sources"); root.append_attribute("xmlns:b")="http://schemas.openxmlformats.org/officeDocument/2006/bibliography"; root.append_attribute("SelectedStyle")=("\\"+m_impl->bibliographyStyle+".XSL").c_str(); root.append_attribute("StyleName")=m_impl->bibliographyStyle.c_str(); root.append_attribute("Version")="6";
            const char* types[]={"Book","JournalArticle","ConferenceProceedings","Report","InternetSite","Misc"};
            for(const auto& entry:m_impl->sources) {
                const auto& s=entry.second; package.sourceTags.insert(s.tag); auto n=root.append_child("b:Source");
                auto text=[&](const char* name,const std::string& value) {if(!value.empty())n.append_child(name).text().set(value.c_str());};
                text("b:Tag",s.tag); text("b:SourceType",types[static_cast<int>(s.type)]); text("b:LCID",std::to_string(s.language));
                if(!s.authors.empty()) { auto author=n.append_child("b:Author").append_child("b:Author"); bool corp=!s.authors.front().corporate.empty();
                    if(corp) { if(s.authors.size()!=1)throw std::invalid_argument("A corporate source has one corporate author"); author.append_child("b:Corporate").text().set(s.authors.front().corporate.c_str()); }
                    else { auto people=author.append_child("b:NameList"); for(const auto& a:s.authors) { if(!a.corporate.empty())throw std::invalid_argument("Cannot mix corporate and personal authors"); auto p=people.append_child("b:Person"); if(!a.last.empty())p.append_child("b:Last").text().set(a.last.c_str()); if(!a.first.empty())p.append_child("b:First").text().set(a.first.c_str()); if(!a.middle.empty())p.append_child("b:Middle").text().set(a.middle.c_str()); } }
                }
                text("b:Title",s.title); text("b:Year",s.year); text("b:Month",s.month); text("b:Day",s.day); text("b:Publisher",s.publisher); text("b:City",s.city); text("b:JournalName",s.journal); text("b:Volume",s.volume); text("b:Issue",s.issue); text("b:Pages",s.pages); text("b:URL",s.url); text("b:StandardNumber",s.doi); text("b:YearAccessed",s.accessedYear); text("b:MonthAccessed",s.accessedMonth); text("b:DayAccessed",s.accessedDay);
            }
            std::string file="customXml/item1.xml"; int i=1; while(package.parts.count(file))file="customXml/item"+std::to_string(++i)+".xml";
            package.addPart(file,xmlString(b),"application/xml"); package.relate("word/document.xml","../"+file,std::string(relNs())+"customXml");
            auto itemProps="customXml/itemProps"+std::to_string(i)+".xml";
            std::string props="<ds:datastoreItem ds:itemID=\"{A58B05B0-916A-47DB-BCE0-46F2E9A20A00}\" xmlns:ds=\"http://schemas.openxmlformats.org/officeDocument/2006/customXml\"><ds:schemaRefs><ds:schemaRef ds:uri=\"http://schemas.openxmlformats.org/officeDocument/2006/bibliography\"/></ds:schemaRefs></ds:datastoreItem>";
            package.addPart(itemProps,props,"application/vnd.openxmlformats-officedocument.customXmlProperties+xml"); package.relate(file,"itemProps"+std::to_string(i)+".xml",std::string(relNs())+"customXmlProps");
        }
        for(const auto& part:package.parts)if(part.first.find("customXml/")==0&&part.first.size()>4&&part.first.substr(part.first.size()-4)==".xml") {
            pugi::xml_document sources; parseXml(sources,part.second);
            for(auto tag:sources.select_nodes("//*[local-name()='Sources']/*[local-name()='Source']/*[local-name()='Tag']"))package.sourceTags.insert(tag.node().text().get());
        }
        package.process(); package.write(path); result.success=true;
    } catch(const PackageError& e) {result.error={e.code,e.part,e.what()};}
      catch(const std::invalid_argument& e) {result.error={SaveError::InvalidArgument,"",e.what()};}
      catch(const std::exception& e) {result.error={SaveError::IoError,path,e.what()};}
    result.warnings=std::move(package.warnings); return result;
}

bool Document::open(const std::string& path) {
    auto parts=readZip(path); if(!parts.count("word/document.xml")||!parts.count("[Content_Types].xml"))return false;
    try { pugi::xml_document d; parseXml(d,parts.at("word/document.xml")); canonicalizeWordPrefixes(d.document_element()); if(!d.document_element().child("w:body"))return false; } catch(...) {return false;}
    auto replacement=std::make_unique<Impl>(); replacement->templateParts.insert(parts.begin(),parts.end()); replacement->isTemplate=true;
    m_impl=std::move(replacement); addSection(); return true;
}
Document& Document::set(const std::string& key,const std::string& value) { m_impl->vars[key]=value; return *this; }
Document& Document::set(const std::string& key,double value,int precision) { if(precision<0||precision>17||!std::isfinite(value))throw std::invalid_argument("Invalid numeric template value"); std::ostringstream s; s<<std::fixed<<std::setprecision(precision)<<value; return set(key,s.str()); }
Document& Document::set(const std::string& key,Paragraph p) { m_impl->block(key).append(std::move(p)); return *this; }
Document& Document::set(const std::string& key,Table p) { m_impl->block(key).append(std::move(p)); return *this; }
Document& Document::set(const std::string& key,Image p) { m_impl->block(key).append(std::move(p)); return *this; }
Document& Document::set(const std::string& key,BulletList p) { m_impl->block(key).append(std::move(p)); return *this; }
Document& Document::set(const std::string& key,Equation p) { m_impl->block(key).append(std::move(p)); return *this; }
Paragraph& Document::setParagraph(const std::string& key,const std::string& s) { return m_impl->block(key).addParagraph(s); }
Table& Document::setTable(const std::string& key,int r,int c) { return m_impl->block(key).addTable(r,c); }
Image& Document::setImage(const std::string& key,const std::string& path) { return m_impl->block(key).addImage(path); }
BulletList& Document::setBulletList(const std::string& key) { return m_impl->block(key).addBulletList(); }
BulletList& Document::setOrderedList(const std::string& key) { return m_impl->block(key).addOrderedList(); }
Equation& Document::setEquation(const std::string& key,const std::string& s) { return m_impl->block(key).addMath(s,EquationMode::Inline); }
Equation& Document::setDisplayEquation(const std::string& key,const std::string& s) { return m_impl->block(key).addMath(s,EquationMode::Display); }
namespace {
std::string trim(std::string s) { auto first=s.find_first_not_of(" \t\r\n"); if(first==std::string::npos)return ""; return s.substr(first,s.find_last_not_of(" \t\r\n")-first+1); }
void replaceTextNodes(pugi::xml_node p,const std::map<std::string,std::string>& vars) {
    std::vector<pugi::xml_node> nodes; std::vector<size_t> offsets; std::string text;
    for(auto item:p.select_nodes(".//w:t")) { nodes.push_back(item.node()); offsets.push_back(text.size()); text+=item.node().text().get(); }
    struct Replacement { size_t begin,end; std::string value; }; std::vector<Replacement> replacements;
    size_t start=0; while((start=text.find("${",start))!=std::string::npos) { auto end=text.find('}',start+2); if(end==std::string::npos)break; auto it=vars.find(text.substr(start+2,end-start-2)); if(it!=vars.end())replacements.push_back({start,end+1,it->second}); start=end+1; }
    for(auto it=replacements.rbegin();it!=replacements.rend();++it) {
        bool inserted=false;
        for(size_t i=0;i<nodes.size();++i) {
            size_t begin=offsets[i],end=begin+std::string(nodes[i].text().get()).size();
            if(end<=it->begin||begin>=it->end)continue;
            auto s=std::string(nodes[i].text().get()); size_t a=it->begin>begin?it->begin-begin:0,b=std::min(it->end-begin,s.size()); s.replace(a,b-a,inserted?"":it->value); inserted=true; nodes[i].text().set(s.c_str()); if(!nodes[i].attribute("xml:space"))nodes[i].append_attribute("xml:space")="preserve";
        }
    }
}
}
std::string Document::renderXml(const std::string& xml) {
    if(xml.find('$')==std::string::npos&&xml.find('{')==std::string::npos)return xml;
    pugi::xml_document d; parseXml(d,xml); auto root=d.document_element(); canonicalizeWordPrefixes(root);
    if(!root.attribute("xmlns:xw"))root.append_attribute("xmlns:xw")="urn:xword:internal";
    // Conditional blocks are sibling blocks. Nested conditions are rejected explicitly.
    std::function<void(pugi::xml_node)> conditions=[&](pugi::xml_node parent) {
        bool active=false,truth=false,inElse=false;
        for(auto n=parent.first_child();n;) {
            auto next=n.next_sibling(); std::string t=std::string(n.name())=="w:p"?trim(textContent(n)):"";
            if(t.find("{%if ")==0&&t.size()>8&&t.substr(t.size()-2)=="%}") {
                if(active)throw std::invalid_argument("Nested template conditions are unsupported"); auto key=trim(t.substr(5,t.size()-7)); auto it=m_impl->vars.find(key); truth=m_impl->blocks.count(key)&&!m_impl->blocks.at(key)->empty(); if(it!=m_impl->vars.end())truth=truth||(!it->second.empty()&&it->second!="0"&&it->second!="false"); active=true; inElse=false; parent.remove_child(n);
            } else if(t=="{%else%}") { if(!active||inElse)throw std::invalid_argument("Unmatched template else"); inElse=true; parent.remove_child(n); }
            else if(t=="{%endif%}") { if(!active)throw std::invalid_argument("Unmatched template endif"); active=false; parent.remove_child(n); }
            else if(active&&!(inElse?!truth:truth))parent.remove_child(n);
            else conditions(n);
            n=next;
        }
        if(active)throw std::invalid_argument("Unclosed template condition");
    };
    conditions(root);
    auto paragraphs=root.select_nodes(".//w:p");
    for(auto item:paragraphs) {
        auto p=item.node(); std::string text=textContent(p),t=trim(text); bool block=false;
        for(const auto& entry:m_impl->blocks) {
            std::string token="${"+entry.first+"}"; if(text.find(token)==std::string::npos)continue;
            if(t!=token)throw std::invalid_argument("Block placeholder must occupy an entire paragraph: "+entry.first);
            auto parent=p.parent(); pugi::xml_document fragment; parseXml(fragment,documentXml("xw:root",entry.second->toXml()));
            for(auto n:fragment.document_element().children())parent.insert_copy_before(n,p);
            auto sect=p.child("w:pPr").child("w:sectPr");
            if(sect) { auto carry=parent.insert_child_before("w:p",p); carry.append_child("w:pPr").append_copy(sect); }
            parent.remove_child(p); block=true; break;
        }
        if(!block)replaceTextNodes(p,m_impl->vars);
    }
    return xmlString(d);
}
} // namespace xword
