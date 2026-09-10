#pragma once
#include "Paragraph.hpp"
namespace xword {
class BulletList {
public:
    explicit BulletList(ListType type = ListType::Bullet); ~BulletList();
    BulletList(BulletList&&) noexcept; BulletList& operator=(BulletList&&) noexcept;
    BulletList& addItem(const std::string& text);
    Paragraph& addItemParagraph(const std::string& text = "",int level = -1);
    BulletList& setLevel(int level);
    BulletList& setNumId(int id);
    BulletList& setStart(int start);
    BulletList& setFormat(NumberFormat format,const std::string& text = "%1.");
    BulletList& continueFrom(const BulletList& previous);
    int numId() const;
    ListType type() const;
    std::string toXml() const;
private:
    struct Impl; std::unique_ptr<Impl> m_impl;
};
}
