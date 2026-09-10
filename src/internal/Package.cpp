#include "Package.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <regex>
#include <random>
#include <cctype>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

namespace xword { namespace internal {
namespace {
std::string relPath(const std::string& part) {
    if(part.empty()) return "_rels/.rels";
    auto p=std::filesystem::u8path(part);
    return (p.parent_path()/"_rels"/(p.filename().u8string()+".rels")).generic_u8string();
}
pugi::xml_node insertXml(pugi::xml_node parent,pugi::xml_node before,const std::string& xml) {
    pugi::xml_document f; auto r=f.load_string(xml.c_str(),pugi::parse_default|pugi::parse_fragment|pugi::parse_ws_pcdata);
    if(!r)throw std::runtime_error("Invalid generated fragment: "+std::string(r.description()));
    pugi::xml_node first;
    for(auto n:f.children()) { auto c=before?parent.insert_copy_before(n,before):parent.append_copy(n); if(!first)first=c; }
    return first;
}
std::string readFile(const std::string& path) {
    std::ifstream f(std::filesystem::u8path(path),std::ios::binary);
    if(!f)throw std::runtime_error("Cannot read resource: "+path);
    return {std::istreambuf_iterator<char>(f),std::istreambuf_iterator<char>()};
}
void orderProperties(pugi::xml_node parent) {
    static const std::map<std::string,std::vector<std::string>> orders={
        {"w:pPr",{"w:pStyle","w:keepNext","w:keepLines","w:pageBreakBefore","w:framePr","w:widowControl","w:numPr","w:suppressLineNumbers","w:pBdr","w:shd","w:tabs","w:suppressAutoHyphens","w:kinsoku","w:wordWrap","w:overflowPunct","w:topLinePunct","w:autoSpaceDE","w:autoSpaceDN","w:bidi","w:adjustRightInd","w:snapToGrid","w:spacing","w:ind","w:contextualSpacing","w:mirrorIndents","w:suppressOverlap","w:jc","w:textDirection","w:textAlignment","w:textboxTightWrap","w:outlineLvl","w:divId","w:cnfStyle","w:rPr","w:sectPr","w:pPrChange"}},
        {"w:rPr",{"w:rStyle","w:rFonts","w:b","w:bCs","w:i","w:iCs","w:caps","w:smallCaps","w:strike","w:dstrike","w:outline","w:shadow","w:emboss","w:imprint","w:noProof","w:snapToGrid","w:vanish","w:webHidden","w:color","w:spacing","w:w","w:kern","w:position","w:sz","w:szCs","w:highlight","w:u","w:effect","w:bdr","w:shd","w:fitText","w:vertAlign","w:rtl","w:cs","w:em","w:lang","w:eastAsianLayout","w:specVanish","w:oMath","w:rPrChange"}}
    };
    for(auto n:parent.children())orderProperties(n);
    auto it=orders.find(parent.name()); if(it==orders.end())return;
    std::vector<pugi::xml_node> nodes; for(auto n:parent.children()) if(n.type()==pugi::node_element)nodes.push_back(n);
    auto index=[&](pugi::xml_node n) { return std::find(it->second.begin(),it->second.end(),n.name())-it->second.begin(); };
    std::stable_sort(nodes.begin(),nodes.end(),[&](auto a,auto b){return index(a)<index(b);});
    for(auto n:nodes)parent.append_move(n);
}
void removeInternal(pugi::xml_node n) {
    for(auto a=n.first_attribute();a;) { auto next=a.next_attribute(); std::string name=a.name(); if(name.find("xw:")==0||name=="xmlns:xw")n.remove_attribute(a); a=next; }
    for(auto c:n.children())removeInternal(c);
}
}
std::string Package::wordType(const std::string& s) { return "application/vnd.openxmlformats-officedocument.wordprocessingml."+s+"+xml"; }
void Package::addPart(const std::string& name,const std::string& data,const std::string& type) { parts[name]=data; types[name]=type; }
std::string Package::relate(const std::string& from,const std::string& target,const std::string& type,bool external) {
    std::string path=relPath(from); pugi::xml_document d;
    if(parts.count(path))parseXml(d,parts.at(path));
    else { auto r=d.append_child("Relationships"); r.append_attribute("xmlns")="http://schemas.openxmlformats.org/package/2006/relationships"; }
    auto root=d.document_element(); std::set<std::string> ids;
    for(auto n:root.children("Relationship")) {
        ids.insert(n.attribute("Id").value());
        if(n.attribute("Target").value()==target&&n.attribute("Type").value()==type&&std::string(n.attribute("TargetMode").value())==(external?"External":""))return n.attribute("Id").value();
    }
    int id=1; while(ids.count("rId"+std::to_string(id)))++id;
    auto n=root.append_child("Relationship"); std::string result="rId"+std::to_string(id);
    n.append_attribute("Id")=result.c_str(); n.append_attribute("Type")=type.c_str(); n.append_attribute("Target")=target.c_str(); if(external)n.append_attribute("TargetMode")="External";
    parts[path]=xmlString(d); return result;
}
std::string Package::registerMedia(const std::string& path,const std::string& part) {
    std::error_code ec; auto p=std::filesystem::u8path(path);
    if(!std::filesystem::is_regular_file(p,ec)) {
        if(options.missingImagesAreErrors)throw PackageError(SaveError::MissingResource,part,"Missing image: "+path);
        warnings.push_back({SaveError::MissingResource,part,"Skipped missing image: "+path}); return "";
    }
    std::string key=std::filesystem::absolute(p).lexically_normal().u8string(),name;
    if(media.count(key))name=media.at(key);
    else {
        auto ext=p.extension().u8string(); std::transform(ext.begin(),ext.end(),ext.begin(),[](unsigned char c){return static_cast<char>(std::tolower(c));});
        auto type=imageContentType(ext.size()>0?ext.substr(1):"");
        if(type=="application/octet-stream")throw PackageError(SaveError::MissingResource,part,"Unsupported image type: "+path);
        do { name="word/media/xword_"+std::to_string(nextMedia++)+ext; } while(parts.count(name));
        addPart(name,readFile(path),type); media[key]=name;
    }
    return relate(part,std::filesystem::u8path(name).lexically_relative(std::filesystem::u8path(part).parent_path()).generic_u8string(),std::string(relNs())+"image");
}
void Package::caption(pugi::xml_node parent,pugi::xml_node before,const std::string& text,const std::string& target,int kind,bool isTable,bool mainStory) {
    bool numbered=mainStory&&(isTable?tableNumbering:imageNumbering),byChapter=isTable?tableByChapter:imageByChapter;
    auto prefix=isTable?tablePrefix:imagePrefix; std::string seq=isTable?"Table":"Figure";
    if(text.empty()&&target.empty()&&!numbered)return;
    std::string x="<w:p><w:pPr><w:pStyle w:val=\"Caption\"/>";
    if(isTable)x+="<w:keepNext/>";
    x+="<w:keepLines/></w:pPr>";
    auto start=[&](const std::string& name,int k) { return "<xw:bookmarkStart name=\""+xmlEscape(name)+"\" kind=\""+std::to_string(k)+"\"/>"; };
    auto end=[&](const std::string& name) { return "<xw:bookmarkEnd name=\""+xmlEscape(name)+"\"/>"; };
    if(!target.empty())x+=start(target,kind);
    if(numbered) {
        int count=isTable?++table:++figure;
        x+="<w:r><w:t xml:space=\"preserve\">"+xmlEscape(prefix)+" </w:t></w:r>";
        if(!target.empty())x+=start(target+"_n",static_cast<int>(TargetKind::Bookmark));
        if(byChapter) x+=fieldXml("STYLEREF 1 \\s",std::to_string(chapter))+"<w:r><w:t>-</w:t></w:r>";
        x+=fieldXml("SEQ "+seq+(byChapter?" \\s 1":"")+" \\* ARABIC",std::to_string(count));
        if(!target.empty())x+=end(target+"_n");
        x+="<w:r><w:t xml:space=\"preserve\"> </w:t></w:r>";
    }
    x+="<w:r><w:t xml:space=\"preserve\">"+xmlEscape(text)+"</w:t></w:r>";
    if(!target.empty())x+=end(target);
    x+="</w:p>"; insertXml(parent,before,x);
}
void Package::image(pugi::xml_node n,const std::string& part,int width) {
    std::string path=n.attribute("path").value(); auto rid=registerMedia(path,part); if(rid.empty()) { n.parent().remove_child(n); return; }
    int cx=n.attribute("cx").as_int(),cy=n.attribute("cy").as_int();
    if(cx<=0||cy<=0) { auto size=computeImageSize(path,n.attribute("width").as_int(),n.attribute("height").as_int(),width*635); cx=size.widthEmu; cy=size.heightEmu; }
    if(cx>width*635) { cy=static_cast<int>(static_cast<double>(cy)*width*635/cx); cx=width*635; }
    if(cx<=0||cy<=0)throw PackageError(SaveError::InvalidArgument,part,"Invalid image dimensions");
    std::string svgRid; auto ext=std::filesystem::u8path(path).extension().u8string(); std::transform(ext.begin(),ext.end(),ext.begin(),[](unsigned char c){return static_cast<char>(std::tolower(c));});
    if(ext==".svg"&&!std::string(n.attribute("fallback").value()).empty()) {
        svgRid=rid; rid=registerMedia(n.attribute("fallback").value(),part);
        if(rid.empty()) { rid=svgRid; svgRid.clear(); }
    }
    int wrap=n.attribute("wrap").as_int(); bool in=n.attribute("inline").as_bool();
    if(in&&!std::string(n.attribute("caption").value()).empty())throw PackageError(SaveError::InvalidArgument,part,"Inline images cannot carry a block caption");
    auto parent=n.parent(); pugi::xml_node p;
    if(!in) { p=parent.insert_child_before("w:p",n); auto pr=p.append_child("w:pPr"); if(!std::string(n.attribute("caption").value()).empty()||imageNumbering)pr.append_child("w:keepNext"); child(pr,"w:jc","w:val",n.attribute("align").value()); }
    else p=parent;
    auto run=in?parent.insert_child_before("w:r",n):p.append_child("w:r"); auto drawing=run.append_child("w:drawing"); auto container=drawing.append_child(wrap==0?"wp:inline":"wp:anchor");
    for(auto name:{"distT","distB","distL","distR"})container.append_attribute(name)=0;
    if(wrap!=0) {
        container.append_attribute("simplePos")="0"; container.append_attribute("relativeHeight")="0"; container.append_attribute("behindDoc")=wrap==3?"1":"0"; container.append_attribute("locked")="0"; container.append_attribute("layoutInCell")="1"; container.append_attribute("allowOverlap")="1";
        auto simple=container.append_child("wp:simplePos"); simple.append_attribute("x")=0; simple.append_attribute("y")=0;
        const char* refs[]={"page","margin","column","paragraph"}; int h=n.attribute("horizontal").as_int(),v=n.attribute("vertical").as_int();
        if(h==3||v==2)throw PackageError(SaveError::InvalidArgument,part,"Unsupported image position reference on this axis");
        auto hp=container.append_child("wp:positionH"); hp.append_attribute("relativeFrom")=refs[h]; hp.append_child("wp:posOffset").text().set(n.attribute("x").value());
        auto vp=container.append_child("wp:positionV"); vp.append_attribute("relativeFrom")=refs[v]; vp.append_child("wp:posOffset").text().set(n.attribute("y").value());
    }
    auto extent=container.append_child("wp:extent"); extent.append_attribute("cx")=cx; extent.append_attribute("cy")=cy;
    if(wrap!=0) { auto w=container.append_child(wrap==1?"wp:wrapSquare":wrap==2?"wp:wrapTopAndBottom":"wp:wrapNone"); if(wrap==1)w.append_attribute("wrapText")="bothSides"; }
    int id=nextDrawing++; auto dp=container.append_child("wp:docPr"); dp.append_attribute("id")=id; dp.append_attribute("name")=("Picture "+std::to_string(id)).c_str(); dp.append_attribute("descr")=n.attribute("alt").value(); dp.append_attribute("title")=n.attribute("title").value();
    auto locks=container.append_child("wp:cNvGraphicFramePr").append_child("a:graphicFrameLocks"); locks.append_attribute("noChangeAspect")=n.attribute("aspect").as_bool()?"1":"0";
    auto data=container.append_child("a:graphic").append_child("a:graphicData"); data.append_attribute("uri")="http://schemas.openxmlformats.org/drawingml/2006/picture";
    auto pic=data.append_child("pic:pic"); auto nv=pic.append_child("pic:nvPicPr"); auto cn=nv.append_child("pic:cNvPr"); cn.append_attribute("id")=id; cn.append_attribute("name")=dp.attribute("name").value(); nv.append_child("pic:cNvPicPr");
    auto fill=pic.append_child("pic:blipFill"); auto blip=fill.append_child("a:blip"); blip.append_attribute("r:embed")=rid.c_str();
    if(!svgRid.empty()) { auto e=blip.append_child("a:extLst").append_child("a:ext"); e.append_attribute("uri")="{96DAC541-7B7A-43D3-8B79-37D633B846F1}"; auto svg=e.append_child("asvg:svgBlip"); svg.append_attribute("xmlns:asvg")="http://schemas.microsoft.com/office/drawing/2016/SVG/main"; svg.append_attribute("r:embed")=svgRid.c_str(); }
    auto crop=fill.append_child("a:srcRect"); for(auto side:{"l","t","r","b"})crop.append_attribute(side)=n.attribute(side).as_int();
    fill.append_child("a:stretch").append_child("a:fillRect"); auto sp=pic.append_child("pic:spPr"); auto xf=sp.append_child("a:xfrm"); auto off=xf.append_child("a:off"); off.append_attribute("x")=0; off.append_attribute("y")=0; auto e=xf.append_child("a:ext"); e.append_attribute("cx")=cx; e.append_attribute("cy")=cy; auto geom=sp.append_child("a:prstGeom"); geom.append_attribute("prst")="rect"; geom.append_child("a:avLst");
    if(!in)caption(parent,n,n.attribute("caption").value(),n.attribute("target").value(),n.attribute("kind").as_int(),false,part=="word/document.xml");
    parent.remove_child(n);
}
void Package::list(pugi::xml_node n,const std::string& part,int width) {
    std::string key=n.attribute("key").value(); int id=n.attribute("id").as_int();
    auto root=numbering.document_element();
    if(id==0&&lists.count(key))id=lists.at(key);
    if(!id)id=nextNum++;
    bool exists=false; for(auto v:root.children("w:num"))if(v.attribute("w:numId").as_int()==id)exists=true;
    if(!exists) {
        int aid=nextAbstract++; auto a=root.prepend_child("w:abstractNum"); a.append_attribute("w:abstractNumId")=aid; child(a,"w:multiLevelType","w:val","multilevel");
        for(int i=0;i<9;++i) {
            auto l=a.append_child("w:lvl"); l.append_attribute("w:ilvl")=i; child(l,"w:start","w:val",n.attribute("start").value()); child(l,"w:numFmt","w:val",n.attribute("format").value());
            std::string txt=n.attribute("text").value(); auto pos=txt.find("%1"); if(pos!=std::string::npos)txt.replace(pos,2,"%"+std::to_string(i+1)); child(l,"w:lvlText","w:val",txt); child(l,"w:lvlJc","w:val","left");
            auto ind=l.append_child("w:pPr").append_child("w:ind"); ind.append_attribute("w:left")=720*(i+1); ind.append_attribute("w:hanging")=360;
        }
        auto num=root.append_child("w:num"); num.append_attribute("w:numId")=id; child(num,"w:abstractNumId","w:val",std::to_string(aid));
    }
    lists[key]=id;
    auto parent=n.parent();
    for(auto item:n.children("xw:item")) {
        auto p=item.child("w:p"); auto pr=p.child("w:pPr"); if(!pr)pr=p.prepend_child("w:pPr"); pr.remove_child("w:numPr"); auto np=pr.append_child("w:numPr"); child(np,"w:ilvl","w:val",item.attribute("level").value()); child(np,"w:numId","w:val",std::to_string(id));
        auto inserted=parent.insert_copy_before(p,n); transform(inserted,part,width);
    }
    parent.remove_child(n);
}
void Package::transform(pugi::xml_node parent,const std::string& part,int width) {
    for(auto n=parent.first_child();n;) {
        auto next=n.next_sibling(); std::string tag=n.name();
        if(tag=="xw:section") { transform(n,part,n.attribute("width").as_int(width)); while(n.first_child())parent.insert_move_before(n.first_child(),n); parent.remove_child(n); }
        else if(tag=="xw:image") { auto prev=n.previous_sibling(); image(n,part,width); auto first=prev?prev.next_sibling():parent.first_child(); while(first&&first!=next) { transform(first,part,width); first=first.next_sibling(); } }
        else if(tag=="xw:list")list(n,part,width);
        else if(tag=="xw:bookmarkStart") {
            std::string name=n.attribute("name").value(); if(!std::regex_match(name,std::regex("[A-Za-z_][A-Za-z0-9_]{0,39}")))throw PackageError(SaveError::InvalidReference,part,"Invalid bookmark name: "+name);
            if(bookmarks.count(name))throw PackageError(SaveError::InvalidReference,part,"Duplicate bookmark: "+name);
            int id=nextBookmark++; bookmarks[name]={id,n.attribute("kind").as_int()}; n.set_name("w:bookmarkStart"); n.append_attribute("w:id")=id; n.append_attribute("w:name")=name.c_str(); n.remove_attribute("name"); n.remove_attribute("kind");
        } else if(tag=="xw:bookmarkEnd") {
            // End markers are resolved after all targets, including forward definitions.
        } else if(tag=="w:hyperlink"&&n.attribute("xw:url")) {
            std::string url=n.attribute("xw:url").value();
            if(url.empty())throw PackageError(SaveError::InvalidReference,part,"Empty hyperlink target");
            if(url[0]=='#') n.append_attribute("w:anchor")=url.substr(1).c_str();
            else n.append_attribute("r:id")=relate(part,url,std::string(relNs())+"hyperlink",true).c_str();
            n.remove_attribute("xw:url"); transform(n,part,width);
        } else if(tag=="w:tbl"&&n.attribute("xw:width")) {
            int tw=n.attribute("xw:width").as_int(); if(!tw)tw=width;
            auto prev=n.previous_sibling(); caption(parent,n,n.attribute("xw:caption").value(),n.attribute("xw:target").value(),n.attribute("xw:kind").as_int(),true,part=="word/document.xml");
            auto cap=prev?prev.next_sibling():parent.first_child(); if(cap!=n)transform(cap,part,width);
            for(auto c:n.child("w:tblGrid").children("w:gridCol")) c.append_attribute("w:w")=static_cast<int>(std::lround(tw*c.attribute("xw:ratio").as_double()));
            for(auto row:n.children("w:tr"))for(auto cell:row.children("w:tc")) {
                int cw=static_cast<int>(std::lround(tw*cell.attribute("xw:ratio").as_double())); auto pr=cell.child("w:tcPr"); auto w=pr.prepend_child("w:tcW"); w.append_attribute("w:w")=cw; w.append_attribute("w:type")="dxa"; transform(cell,part,std::max(1,cw-216));
                auto last=cell.last_child(); while(last&&last.type()!=pugi::node_element)last=last.previous_sibling();
                if(std::string(last.name())!="w:p")cell.append_child("w:p");
            }
        } else {
            if(part=="word/document.xml"&&std::string(n.child("w:pPr").child("w:pStyle").attribute("w:val").value())=="Heading1"&&n.child("w:pPr").child("w:numPr")) { ++chapter; if(imageByChapter)figure=0; if(tableByChapter)table=0; }
            transform(n,part,width);
        }
        n=next;
    }
}
void Package::resolve(pugi::xml_node parent,const std::string& part) {
    for(auto n=parent.first_child();n;) {
        auto next=n.next_sibling(); std::string tag=n.name();
        if(tag=="xw:bookmarkEnd") {
            std::string name=n.attribute("name").value(); if(!bookmarks.count(name)||!bookmarkEnds.insert(name).second)throw PackageError(SaveError::InvalidReference,part,"Unmatched bookmark end: "+name);
            n.set_name("w:bookmarkEnd"); n.append_attribute("w:id")=bookmarks.at(name).first; n.remove_attribute("name"); n.remove_attribute("kind");
        } else if(tag=="xw:ref") {
            std::string name=n.attribute("target").value(),opts=n.attribute("options").value();
            int targetKind=0,kind=0,link=1; char sep; std::istringstream stream(opts); stream>>targetKind>>sep>>kind>>sep>>link;
            if(!bookmarks.count(name)||bookmarks.at(name).second!=targetKind)throw PackageError(SaveError::InvalidReference,part,"Unknown or mismatched reference: "+name);
            std::string target=name,sw;
            if(kind==static_cast<int>(ReferenceKind::Number)) {
                if(bookmarks.count(name+"_n"))target=name+"_n";
                else if(targetKind==static_cast<int>(TargetKind::Heading))sw=" \\n";
                else throw PackageError(SaveError::InvalidReference,part,"Target has no number: "+name);
            }
            std::string instruction=(kind==static_cast<int>(ReferenceKind::Page)?"PAGEREF ":"REF ")+target+sw+(link?" \\h":"");
            bool unknown=kind==static_cast<int>(ReferenceKind::Page)||(kind==static_cast<int>(ReferenceKind::Number)&&!sw.empty());
            insertXml(parent,n,fieldXml(instruction,unknown?"":bookmarkText[target])); parent.remove_child(n);
        } else if(tag=="xw:citation") {
            std::string tagName=n.attribute("target").value(),pages=n.attribute("options").value();
            if(!sourceTags.count(tagName))throw PackageError(SaveError::InvalidReference,part,"Unknown bibliography source: "+tagName);
            if(tagName.find_first_of("\"\\\r\n")!=std::string::npos||pages.find_first_of("\"\\\r\n")!=std::string::npos)throw PackageError(SaveError::InvalidArgument,part,"Invalid citation argument");
            insertXml(parent,n,fieldXml("CITATION \""+tagName+"\""+(pages.empty()?"":" \\p \""+pages+"\""))); parent.remove_child(n);
        } else resolve(n,part);
        n=next;
    }
}
void Package::process() {
    std::map<std::string,std::string> originals;
    if(parts.count("word/numbering.xml"))parseXml(numbering,parts.at("word/numbering.xml"));
    else parseXml(numbering,documentXml("w:numbering",""));
    canonicalizeWordPrefixes(numbering.document_element());
    for(auto n:numbering.document_element().children("w:num"))nextNum=std::max(nextNum,n.attribute("w:numId").as_int()+1);
    for(auto n:numbering.document_element().children("w:abstractNum"))nextAbstract=std::max(nextAbstract,n.attribute("w:abstractNumId").as_int()+1);
    for(const auto& entry:parts) {
        if(entry.first.find("word/")!=0||entry.first.size()<4||entry.first.substr(entry.first.size()-4)!=".xml")continue;
        if(entry.first=="word/styles.xml"||entry.first=="word/numbering.xml"||entry.first=="word/settings.xml")continue;
        auto doc=std::make_unique<pugi::xml_document>(); parseXml(*doc,entry.second); canonicalizeWordPrefixes(doc->document_element());
        for(auto item:doc->select_nodes("//wp:docPr"))nextDrawing=std::max(nextDrawing,item.node().attribute("id").as_int()+1);
        for(auto item:doc->select_nodes("//w:bookmarkStart")) {
            auto n=item.node(); std::string name=n.attribute("w:name").value(); int id=n.attribute("w:id").as_int();
            if(bookmarks.count(name))throw PackageError(SaveError::InvalidReference,entry.first,"Duplicate bookmark: "+name);
            bookmarks[name]={id,0}; nextBookmark=std::max(nextBookmark,id+1);
        }
        originals[entry.first]=xmlString(*doc);
        // Newly inserted drawings and links need canonical namespace bindings.
        pugi::xml_document ns; parseXml(ns,documentXml("xw:root",""));
        if(entry.second.find("xw:")!=std::string::npos)for(auto a:ns.document_element().attributes())if(!doc->document_element().attribute(a.name()))doc->document_element().append_attribute(a.name())=a.value();
        stories[entry.first]=std::move(doc);
    }
    if(stories.count("word/document.xml"))transform(*stories.at("word/document.xml"),"word/document.xml",9360);
    for(auto& entry:stories)if(entry.first!="word/document.xml")transform(*entry.second,entry.first,9360);
    for(auto& entry:stories) {
        auto& d=*entry.second;
        // Collect cached bookmark text before resolving cross references.
        for(auto item:d.select_nodes("//w:bookmarkStart")) {
            auto start=item.node(); std::string name=start.attribute("w:name").value(),text;
            for(auto n=start.next_sibling();n;n=n.next_sibling()) {
                if(std::string(n.name())=="xw:bookmarkEnd"&&std::string(n.attribute("name").value())==name)break;
                if(std::string(n.name())=="w:bookmarkEnd"&&std::string(n.attribute("w:id").value())==start.attribute("w:id").value())break;
                text+=textContent(n);
            }
            bookmarkText[name]=text;
        }
    }
    for(auto& entry:stories) { resolve(*entry.second,entry.first); if(types.count(entry.first)||parts[entry.first].find("xw:")!=std::string::npos)orderProperties(*entry.second); removeInternal(*entry.second); auto xml=xmlString(*entry.second); if(xml!=originals.at(entry.first))parts[entry.first]=xml; }
    removeInternal(numbering); addPart("word/numbering.xml",xmlString(numbering),wordType("numbering"));
    relate("word/document.xml","numbering.xml",std::string(relNs())+"numbering");
    // Preserve template content types and append only new or updated parts.
    pugi::xml_document ct; if(parts.count("[Content_Types].xml"))parseXml(ct,parts.at("[Content_Types].xml"));
    else { auto r=ct.append_child("Types"); r.append_attribute("xmlns")="http://schemas.openxmlformats.org/package/2006/content-types"; }
    auto root=ct.document_element();
    auto def=[&](const char* ext,const char* type) { for(auto n:root.children("Default"))if(std::string(n.attribute("Extension").value())==ext)return; auto n=root.append_child("Default"); n.append_attribute("Extension")=ext; n.append_attribute("ContentType")=type; };
    def("rels","application/vnd.openxmlformats-package.relationships+xml"); def("xml","application/xml");
    for(const auto& entry:types) {
        pugi::xml_node found; for(auto n:root.children("Override"))if(std::string(n.attribute("PartName").value())=="/"+entry.first)found=n;
        if(!found) { found=root.append_child("Override"); found.append_attribute("PartName")=("/"+entry.first).c_str(); found.append_attribute("ContentType"); }
        found.attribute("ContentType")=entry.second.c_str();
    }
    parts["[Content_Types].xml"]=xmlString(ct); validate();
}
void Package::validate() {
    std::map<std::string,std::set<std::string>> ids;
    std::set<std::string> footnotes,endnotes,comments,styleIds,numIds;
    for(const auto& entry:parts) {
        auto name=entry.first; bool rel=name.size()>=5&&name.substr(name.size()-5)==".rels";
        if(!rel&&(name.size()<4||name.substr(name.size()-4)!=".xml"))continue;
        pugi::xml_document d; try {parseXml(d,entry.second);canonicalizeWordPrefixes(d.document_element());}catch(const std::exception& e){throw PackageError(SaveError::InvalidXml,name,e.what());}
        if(rel) {
            auto slash=name.find("/_rels/"); std::string from;
            if(name!="_rels/.rels") { if(slash==std::string::npos)throw PackageError(SaveError::InvalidReference,name,"Invalid relationships path"); from=name.substr(0,slash+1)+name.substr(slash+7,name.size()-(slash+7)-5); }
            for(auto r:d.document_element().children("Relationship")) {
                std::string id=r.attribute("Id").value(); if(!ids[from].insert(id).second)throw PackageError(SaveError::InvalidReference,name,"Duplicate relationship ID");
                if(std::string(r.attribute("TargetMode").value())=="External")continue;
                std::string raw=r.attribute("Target").value(); auto hash=raw.find('#'); if(hash!=std::string::npos)raw.resize(hash);
                auto target=(std::filesystem::u8path(from).parent_path()/std::filesystem::u8path(raw)).lexically_normal().generic_u8string(); if(!target.empty()&&target[0]=='/')target.erase(0,1);
                if(!parts.count(target))throw PackageError(SaveError::InvalidReference,name,"Missing relationship target: "+target);
            }
        }
        if(name=="word/footnotes.xml")for(auto n:d.document_element().children("w:footnote"))footnotes.insert(n.attribute("w:id").value());
        if(name=="word/endnotes.xml")for(auto n:d.document_element().children("w:endnote"))endnotes.insert(n.attribute("w:id").value());
        if(name=="word/comments.xml")for(auto n:d.document_element().children("w:comment"))comments.insert(n.attribute("w:id").value());
        if(name=="word/styles.xml")for(auto n:d.document_element().children("w:style"))styleIds.insert(n.attribute("w:styleId").value());
        if(name=="word/numbering.xml")for(auto n:d.document_element().children("w:num"))numIds.insert(n.attribute("w:numId").value());
    }
    std::set<int> drawingIds;
    for(const auto& entry:stories) {
        const auto& name=entry.first; auto& d=*entry.second;
        for(auto a:d.select_nodes("//@r:id | //@r:embed | //@r:link")) if(!ids[name].count(a.attribute().value()))throw PackageError(SaveError::InvalidReference,name,"Unknown relationship: "+std::string(a.attribute().value()));
        for(auto b:d.select_nodes("//w:hyperlink[@w:anchor]"))if(!bookmarks.count(b.node().attribute("w:anchor").value()))throw PackageError(SaveError::InvalidReference,name,"Unknown hyperlink bookmark");
        for(auto item:d.select_nodes("//w:footnoteReference | //w:endnoteReference")) {
            auto n=item.node(); bool end=std::string(n.name())=="w:endnoteReference";
            if(n.attribute("w:id").as_int()<=0||!(end?endnotes:footnotes).count(n.attribute("w:id").value()))throw PackageError(SaveError::InvalidReference,name,"Unknown note reference");
            if(name!="word/document.xml")throw PackageError(SaveError::InvalidReference,name,"Note references are only allowed in the main story");
        }
        std::map<std::string,int> starts,ends,refs;
        std::set<std::string> openComments;
        auto commentEvents=d.select_nodes("//w:commentRangeStart | //w:commentRangeEnd | //w:commentReference"); commentEvents.sort();
        for(auto item:commentEvents) {
            auto n=item.node(); std::string id=n.attribute("w:id").value(),tag=n.name(); if(!comments.count(id)||name!="word/document.xml")throw PackageError(SaveError::InvalidReference,name,"Invalid comment reference");
            if(tag=="w:commentRangeStart") {++starts[id];openComments.insert(id);} else if(tag=="w:commentRangeEnd") {if(!openComments.erase(id))throw PackageError(SaveError::InvalidReference,name,"Comment ends before it starts");++ends[id];} else ++refs[id];
        }
        for(const auto& c:starts)if(c.second!=1||ends[c.first]!=1||refs[c.first]!=1)throw PackageError(SaveError::InvalidReference,name,"Unbalanced comment range");
        if(starts.size()!=ends.size()||starts.size()!=refs.size())throw PackageError(SaveError::InvalidReference,name,"Unbalanced comment range");
        std::map<int,int> bs,be;
        for(auto item:d.select_nodes("//w:bookmarkStart"))++bs[item.node().attribute("w:id").as_int()];
        for(auto item:d.select_nodes("//w:bookmarkEnd"))++be[item.node().attribute("w:id").as_int()];
        if(bs!=be)throw PackageError(SaveError::InvalidReference,name,"Unbalanced bookmark range");
        std::set<int> openBookmarks;
        auto bookmarkEvents=d.select_nodes("//w:bookmarkStart | //w:bookmarkEnd"); bookmarkEvents.sort();
        for(auto item:bookmarkEvents) {
            auto n=item.node(); int id=n.attribute("w:id").as_int();
            if(std::string(n.name())=="w:bookmarkStart")openBookmarks.insert(id);
            else if(!openBookmarks.erase(id))throw PackageError(SaveError::InvalidReference,name,"Bookmark ends before it starts");
        }
        for(const auto& b:bs)if(b.second!=1)throw PackageError(SaveError::InvalidReference,name,"Duplicate bookmark ID");
        for(auto item:d.select_nodes("//wp:docPr"))if(!drawingIds.insert(item.node().attribute("id").as_int()).second)throw PackageError(SaveError::InvalidReference,name,"Duplicate drawing ID");
        for(auto item:d.select_nodes("//w:pStyle | //w:rStyle | //w:tblStyle"))if(!styleIds.count(item.node().attribute("w:val").value()))throw PackageError(SaveError::InvalidReference,name,"Unknown style: "+std::string(item.node().attribute("w:val").value()));
        for(auto item:d.select_nodes("//w:numId"))if(item.node().attribute("w:val").as_int()!=0&&!numIds.count(item.node().attribute("w:val").value()))throw PackageError(SaveError::InvalidReference,name,"Unknown numbering ID");
        if(!d.select_nodes("//xw:* | //@xw:*").empty())throw PackageError(SaveError::InvalidXml,name,"Unresolved internal node");
        if(!d.select_nodes("//w:tc//w:sectPr").empty())throw PackageError(SaveError::InvalidXml,name,"Section properties inside a table cell");
    }
}
void Package::write(const std::string& path) {
    namespace fs=std::filesystem; fs::path target=fs::absolute(fs::u8path(path));
    std::random_device rd; fs::path temp=target; temp+=L".xword-"+std::to_wstring(rd())+L".tmp";
    try {
        { ZipWriter zip(temp.u8string()); for(const auto& e:parts)if(!zip.addEntry(e.first,e.second))throw PackageError(SaveError::IoError,e.first,"ZIP entry write failed"); if(!zip.finalize())throw PackageError(SaveError::IoError,path,"ZIP finalization failed"); }
#ifdef _WIN32
        if(!MoveFileExW(temp.c_str(),target.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))throw PackageError(SaveError::IoError,path,"Cannot replace output file (Windows error "+std::to_string(GetLastError())+")");
#else
        fs::rename(temp,target);
#endif
    } catch(...) { std::error_code ec; fs::remove(temp,ec); throw; }
}
}}
