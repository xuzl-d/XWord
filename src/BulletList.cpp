#include "xword/BulletList.hpp"
#include "internal/ZipWriter.hpp"
#include <vector>
#include <atomic>
namespace xword {
struct BulletList::Impl {
    ListType type; int level=0,id=0,start=1; unsigned long long key=0;
    NumberFormat format=NumberFormat::Decimal; std::string text="%1.";
    struct Item { int level; std::unique_ptr<Paragraph> paragraph; };
    std::vector<Item> items;
};
BulletList::BulletList(ListType t):m_impl(std::make_unique<Impl>()) { static std::atomic<unsigned long long> next{1}; m_impl->key=next++; m_impl->type=t; if(t==ListType::Bullet) {m_impl->format=NumberFormat::Bullet; m_impl->text=u8"•";} }
BulletList::~BulletList()=default; BulletList::BulletList(BulletList&&) noexcept=default; BulletList& BulletList::operator=(BulletList&&) noexcept=default;
BulletList& BulletList::addItem(const std::string& s) { addItemParagraph(s); return *this; }
Paragraph& BulletList::addItemParagraph(const std::string& s,int level) { if(level<0) level=m_impl->level; if(level>8) throw std::invalid_argument("List level must be 0..8"); auto p=std::make_unique<Paragraph>(); if(!s.empty()) p->addRun(s); p->setStyleId("ListParagraph"); auto result=p.get(); m_impl->items.push_back({level,std::move(p)}); return *result; }
BulletList& BulletList::setLevel(int n) { if(n<0||n>8) throw std::invalid_argument("List level must be 0..8"); m_impl->level=n; return *this; }
BulletList& BulletList::setNumId(int n) { if(n<1) throw std::invalid_argument("Invalid numbering id"); m_impl->id=n; return *this; }
BulletList& BulletList::setStart(int n) { if(n<1) throw std::invalid_argument("Invalid list start"); m_impl->start=n; return *this; }
BulletList& BulletList::setFormat(NumberFormat n,const std::string& s) { m_impl->format=n; m_impl->text=s; return *this; }
BulletList& BulletList::continueFrom(const BulletList& p) { m_impl->key=p.m_impl->key; m_impl->id=p.m_impl->id; m_impl->format=p.m_impl->format; m_impl->text=p.m_impl->text; m_impl->start=p.m_impl->start; return *this; }
int BulletList::numId() const { return m_impl->id; } ListType BulletList::type() const { return m_impl->type; }
std::string BulletList::toXml() const {
    std::string x="<xw:list key=\""+std::to_string(m_impl->key)+"\" id=\""+std::to_string(m_impl->id)+"\" start=\""+std::to_string(m_impl->start)+"\" format=\""+numberFormatName(m_impl->format)+"\" text=\""+internal::xmlEscape(m_impl->text)+"\">";
    for(const auto& i:m_impl->items) x+="<xw:item level=\""+std::to_string(i.level)+"\">"+i.paragraph->toXml()+"</xw:item>";
    return x+"</xw:list>";
}
}
