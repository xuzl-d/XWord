#pragma once

#include "Types.hpp"
#include <memory>
#include <string>

namespace xword {

/// A bullet or ordered (numbered) list.
///
/// Created via Document::addBulletList() or Document::addOrderedList().
/// Items are added with addItem() and support indentation levels.
class BulletList {
public:
    BulletList(ListType type = ListType::Bullet);
    ~BulletList();
    BulletList(BulletList&&) noexcept;
    BulletList& operator=(BulletList&&) noexcept;

    /// @{
    /// Builder methods (chainable).

    /// Append an item to the list.
    BulletList& addItem(const std::string& text);

    /// Set the nesting level (0 = top level).
    BulletList& setLevel(int level);

    /// Assign a numbering definition ID.
    BulletList& setNumId(int id);
    /// @}

    /// Build OOXML list XML (internal use).
    std::string toXml() const;

    /// The numbering definition ID in use.
    int numId() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace xword
