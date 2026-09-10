#pragma once
#include "pugixml.hpp"
#include "ZipWriter.hpp"
#include <sstream>
#include <stdexcept>
#include <map>
#include <functional>
namespace xword { namespace internal {
inline const char* wordNs() { return "http://schemas.openxmlformats.org/wordprocessingml/2006/main"; }
inline const char* relNs() { return "http://schemas.openxmlformats.org/officeDocument/2006/relationships/"; }
inline std::string namespaces() {
    return " xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\" xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\" xmlns:xw=\"urn:xword:internal\"";
}
inline std::string documentXml(const std::string& tag,const std::string& body) { return "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><"+tag+namespaces()+">"+body+"</"+tag+">"; }
inline std::string xmlString(pugi::xml_node node) { std::ostringstream s; node.print(s,"",pugi::format_raw,pugi::encoding_utf8); return s.str(); }
inline void parseXml(pugi::xml_document& doc,const std::string& s) { validateXmlCharacters(s); auto r=doc.load_buffer(s.data(),s.size(),pugi::parse_default|pugi::parse_ws_pcdata,pugi::encoding_utf8); if(!r) throw std::runtime_error("Invalid XML: "+std::string(r.description())+" at "+std::to_string(r.offset)); }
inline void appendXml(pugi::xml_node parent,const std::string& s) { pugi::xml_document f; auto r=f.load_string(s.c_str(),pugi::parse_default|pugi::parse_fragment|pugi::parse_ws_pcdata); if(!r) throw std::runtime_error("Invalid XML fragment: "+std::string(r.description())); for(auto n:f.children()) parent.append_copy(n); }
inline pugi::xml_node child(pugi::xml_node p,const char* name,const char* attribute,const std::string& value) { auto n=p.append_child(name); n.append_attribute(attribute).set_value(value.c_str()); return n; }
inline std::string fieldXml(const std::string& instruction,const std::string& cached="") {
    return "<w:r><w:fldChar w:fldCharType=\"begin\" w:dirty=\"true\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> "+xmlEscape(instruction)+" </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r><w:r><w:t xml:space=\"preserve\">"+xmlEscape(cached)+"</w:t></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
}
inline std::string textContent(pugi::xml_node node) { std::string s; for(auto n:node.select_nodes(".//w:t")) s+=n.node().text().get(); return s; }
inline void canonicalizeWordPrefixes(pugi::xml_node root) {
    const std::map<std::string,std::string> canonical={
        {wordNs(),"w"}, {"http://schemas.openxmlformats.org/officeDocument/2006/relationships","r"},
        {"http://schemas.openxmlformats.org/officeDocument/2006/math","m"},
        {"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing","wp"},
        {"http://schemas.openxmlformats.org/drawingml/2006/main","a"},
        {"http://schemas.openxmlformats.org/drawingml/2006/picture","pic"}
    };
    std::map<std::string,std::string> used;
    std::function<void(pugi::xml_node,std::map<std::string,std::string>)> walk=[&](pugi::xml_node n,std::map<std::string,std::string> ns) {
        for(auto a:n.attributes()) { std::string name=a.name(); if(name=="xmlns")ns[""]=a.value(); else if(name.find("xmlns:")==0)ns[name.substr(6)]=a.value(); }
        auto rename=[&](const std::string& name,bool attribute) {
            auto pos=name.find(':'); if(pos==std::string::npos&&attribute)return name;
            std::string prefix=pos==std::string::npos?"":name.substr(0,pos),local=pos==std::string::npos?name:name.substr(pos+1);
            auto it=ns.find(prefix); if(it==ns.end())return name; auto target=canonical.find(it->second); if(target==canonical.end())return name;
            used[target->second]=it->second; return target->second+":"+local;
        };
        if(n.type()==pugi::node_element)n.set_name(rename(n.name(),false).c_str());
        for(auto a:n.attributes()) { std::string name=a.name(); if(name!="xmlns"&&name.find("xmlns:")!=0)a.set_name(rename(name,true).c_str()); }
        for(auto child:n.children())walk(child,ns);
    };
    walk(root,{});
    for(const auto& e:used)if(!root.attribute(("xmlns:"+e.first).c_str()))root.append_attribute(("xmlns:"+e.first).c_str())=e.second.c_str();
}
}}
