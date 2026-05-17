#include "xword/BulletList.hpp"
#include "internal/ZipWriter.hpp"
#include <vector>

namespace xword {

struct BulletList::Impl {
    ListType m_type;
    int m_level = 0;
    int m_numId = 0;
    std::vector<std::string> m_items;
};

BulletList::BulletList(ListType type) : m_impl(std::make_unique<Impl>()) {
    m_impl->m_type = type;
}

BulletList::~BulletList() = default;
BulletList::BulletList(BulletList&&) noexcept = default;
BulletList& BulletList::operator=(BulletList&&) noexcept = default;

BulletList& BulletList::addItem(const std::string& text) {
    m_impl->m_items.push_back(text);
    return *this;
}

BulletList& BulletList::setLevel(int level) {
    m_impl->m_level = level;
    return *this;
}

BulletList& BulletList::setNumId(int id) {
    m_impl->m_numId = id;
    return *this;
}

std::string BulletList::toXml() const {
    std::string xml;
    int numId = m_impl->m_numId > 0 ? m_impl->m_numId : ((m_impl->m_type == ListType::Bullet) ? 1 : 2);
    for (const auto& item : m_impl->m_items) {
        xml += "<w:p>"
               "<w:pPr>"
               "<w:pStyle w:val=\"ListParagraph\"/>"
               "<w:numPr>"
               "<w:ilvl w:val=\"" + std::to_string(m_impl->m_level) + "\"/>"
               "<w:numId w:val=\"" + std::to_string(numId) + "\"/>"
               "</w:numPr>"
               "</w:pPr>"
               "<w:r><w:t xml:space=\"preserve\">" + internal::xmlEscape(item) + "</w:t></w:r>"
               "</w:p>";
    }
    return xml;
}

int BulletList::numId() const {
    return m_impl->m_numId;
}

} // namespace xword
