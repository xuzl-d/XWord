#include "xword/Content.hpp"
#include "xword/Table.hpp"
#include "xword/BulletList.hpp"
#include "internal/utf.hpp"
#include <variant>
#include <vector>

namespace xword {
struct Content::Impl {
    using Block=std::variant<std::unique_ptr<Paragraph>,std::unique_ptr<Table>,std::unique_ptr<Image>,std::unique_ptr<BulletList>,std::unique_ptr<Equation>>;
    std::vector<Block> blocks;
    RunStyle style;
    const RunStyle* parentStyle = nullptr;
};
Content::Content():m_impl(std::make_unique<Impl>()) {}
Content::~Content()=default;
Content::Content(Content&&) noexcept=default;
Content& Content::operator=(Content&&) noexcept=default;
Paragraph& Content::addParagraph(const std::string& s) {
    auto p=std::make_unique<Paragraph>(); p->setStyle(effectiveStyle()).setFirstLineIndent(0); if(!s.empty()) p->addRun(s);
    auto result=p.get(); m_impl->blocks.emplace_back(std::move(p)); return *result;
}
Paragraph& Content::addParagraph(const std::wstring& s) { return addParagraph(internal::wstring_to_utf8(s)); }
Paragraph& Content::addParagraph(const std::string& s,const RunStyle& r) { auto& p=addParagraph(); return p.addRun(s,r); }
Paragraph& Content::addParagraph(const std::wstring& s,const RunStyle& r) { return addParagraph(internal::wstring_to_utf8(s),r); }
Paragraph& Content::addEquation(const std::string& s) { return addParagraph().addEquation(s); }
Paragraph& Content::addEquation(const std::wstring& s) { return addEquation(internal::wstring_to_utf8(s)); }
Equation& Content::addMath(const std::string& s,EquationMode mode) { auto p=std::make_unique<Equation>(s,mode); auto result=p.get(); m_impl->blocks.emplace_back(std::move(p)); return *result; }
Image& Content::addImage(const std::string& path) { auto p=std::make_unique<Image>(path); auto result=p.get(); m_impl->blocks.emplace_back(std::move(p)); return *result; }
Image& Content::addImage(const std::string& path,int w,int h) { return addImage(path).setSize(w,h); }
// The remaining overloads normalise the path and funnel into the UTF-8 entry point.
Image& Content::addImage(const char* path) { return addImage(std::string(path)); }
Image& Content::addImage(const std::wstring& path) { return addImage(std::filesystem::path(path).u8string()); }
Image& Content::addImage(const std::filesystem::path& path) { return addImage(path.u8string()); }
Image& Content::addImage(const std::wstring& path,int w,int h) { return addImage(std::filesystem::path(path).u8string(),w,h); }
Image& Content::addImage(const std::filesystem::path& path,int w,int h) { return addImage(path.u8string(),w,h); }
Table& Content::addTable(int r,int c) { auto p=std::make_unique<Table>(r,c); p->setStyle(effectiveStyle()); auto result=p.get(); m_impl->blocks.emplace_back(std::move(p)); return *result; }
BulletList& Content::addBulletList() { auto p=std::make_unique<BulletList>(); auto result=p.get(); m_impl->blocks.emplace_back(std::move(p)); return *result; }
BulletList& Content::addOrderedList() { auto p=std::make_unique<BulletList>(ListType::Ordered); auto result=p.get(); m_impl->blocks.emplace_back(std::move(p)); return *result; }
void Content::setStyle(const RunStyle& s) { m_impl->style=s; }
void Content::inheritStyle(const RunStyle* s) { m_impl->parentStyle=s; }
RunStyle Content::effectiveStyle() const { return m_impl->parentStyle ? m_impl->style.withDefaults(*m_impl->parentStyle) : m_impl->style; }
RunStyle& Content::getStyle() { return m_impl->style; }
bool Content::empty() const { return m_impl->blocks.empty(); }
std::string Content::toXml() const {
    std::string x;
    for(const auto& b:m_impl->blocks) std::visit([&](const auto& p){
        using T=typename std::decay_t<decltype(p)>::element_type;
        if constexpr(std::is_same_v<T,Equation>) x+="<w:p>"+p->toXml()+"</w:p>";
        else x+=p->toXml();
    },b);
    return x;
}
void Content::append(Paragraph p) { m_impl->blocks.emplace_back(std::make_unique<Paragraph>(std::move(p))); }
void Content::append(Table p) { m_impl->blocks.emplace_back(std::make_unique<Table>(std::move(p))); }
void Content::append(Image p) { m_impl->blocks.emplace_back(std::make_unique<Image>(std::move(p))); }
void Content::append(BulletList p) { m_impl->blocks.emplace_back(std::make_unique<BulletList>(std::move(p))); }
void Content::append(Equation p) { m_impl->blocks.emplace_back(std::make_unique<Equation>(std::move(p))); }

Note::Note(int id) : id_(id) {}
int Note::id() const { return id_; }
}
