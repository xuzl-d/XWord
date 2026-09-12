#pragma once

#include "Paragraph.hpp"

namespace xword
{

/// A bullet or ordered (numbered) list.
///
/// Lists are created through Document::addBulletList() / addOrderedList().
/// Numbering instances are allocated at save time, so several lists can share
/// one sequence by passing the same numId, or continue an earlier list through
/// continueFrom().
///
///     auto& list = doc.addBulletList();
///     list.addItem("First point").addItem("Second point");
class BulletList
{
public:
    /// @param type  Bullet (default) or Ordered.
    explicit BulletList(ListType type = ListType::Bullet);

    /// @name Rule of five
    /// A list owns its items: movable, not copyable.
    /// @{
    ~BulletList();
    BulletList(BulletList&&) noexcept;
    BulletList& operator=(BulletList&&) noexcept;
    /// @}

    /// Append an item holding plain text.
    BulletList& addItem(const std::string& text);

    /// Append an item and return its paragraph for richer content.
    /// @param text   Initial text; empty for a blank item.
    /// @param level  Nesting level, or -1 to use the list's current level.
    Paragraph& addItemParagraph(const std::string& text = "", int level = -1);

    /// Set the nesting level used by subsequent items (0-based).
    BulletList& setLevel(int level);

    /// Use a specific numbering instance, so several lists share one sequence.
    BulletList& setNumId(int id);

    /// First number of an ordered list.
    BulletList& setStart(int start);

    /// Number format and label pattern, e.g. "%1." for "1." or "(%1)".
    BulletList& setFormat(NumberFormat format, const std::string& text = "%1.");

    /// Continue the numbering of an earlier list.
    BulletList& continueFrom(const BulletList& previous);

    /// The numbering instance in use, or 0 when not yet assigned.
    int numId() const;

    /// The list kind this list was created with.
    ListType type() const;

    /// Build the OOXML for this list (internal use).
    std::string toXml() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};
} // namespace xword
