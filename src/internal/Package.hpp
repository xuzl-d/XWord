#pragma once
#include "Xml.hpp"
#include "xword/Types.hpp"
#include <map>
#include <set>
#include <memory>
namespace xword { namespace internal {
struct PackageError : std::runtime_error {
    SaveError code; std::string part;
    PackageError(SaveError c,const std::string& p,const std::string& s):std::runtime_error(s),code(c),part(p) {}
};
class Package {
public:
    std::map<std::string,std::string> parts;
    SaveOptions options;
    std::vector<Diagnostic> warnings;
    std::set<std::string> sourceTags;
    bool imageNumbering=false,tableNumbering=false;
    bool imageByChapter=false,tableByChapter=false;
    std::string imagePrefix=u8"图",tablePrefix=u8"表";
    void addPart(const std::string& name,const std::string& data,const std::string& contentType);
    std::string relate(const std::string& from,const std::string& target,const std::string& type,bool external=false);
    void process();
    void write(const std::string& path);
    static std::string wordType(const std::string& suffix);
private:
    std::map<std::string,std::string> types,media;
    std::map<std::string,int> lists;
    std::map<std::string,std::pair<int,int>> bookmarks;
    std::map<std::string,std::string> bookmarkText;
    std::set<std::string> bookmarkEnds;
    int nextDrawing=1,nextMedia=1,nextBookmark=0,nextNum=100,nextAbstract=100;
    int chapter=0,figure=0,table=0;
    pugi::xml_document numbering;
    std::map<std::string,std::unique_ptr<pugi::xml_document>> stories;
    void transform(pugi::xml_node parent,const std::string& part,int width);
    void resolve(pugi::xml_node parent,const std::string& part);
    void image(pugi::xml_node node,const std::string& part,int width);
    void list(pugi::xml_node node,const std::string& part,int width);
    void caption(pugi::xml_node parent,pugi::xml_node before,const std::string& text,const std::string& target,int kind,bool isTable,bool mainStory);
    std::string registerMedia(const std::string& path,const std::string& part);
    void validate();
};
}}
