#pragma once

#include <filesystem>
#include <memory>

#include "Equation.hpp"
#include "Paragraph.hpp"

namespace xword
{
class Table;
class BulletList;

/// Ordered content owned by its parent. References remain valid until parent destruction.
class Content
{
public:
    Content();

    /// @name Rule of five
    /// Content owns the blocks added to it: movable, not copyable.
    /// @{
    virtual ~Content();
    Content(Content&&) noexcept;
    Content& operator=(Content&&) noexcept;
    Content(const Content&)            = delete;
    Content& operator=(const Content&) = delete;
    /// @}

    /// Append an empty paragraph, or one holding @p text.
    /// @return Reference for chaining.
    Paragraph& addParagraph(const std::string& text = "");
    Paragraph& addParagraph(const std::wstring& text);

    /// Append a paragraph whose runs default to @p style.
    Paragraph& addParagraph(const std::string& text, const RunStyle& style);
    Paragraph& addParagraph(const std::wstring& text, const RunStyle& style);

    /// Append a paragraph holding one inline equation.
    Paragraph& addEquation(const std::string& latex);
    Paragraph& addEquation(const std::wstring& latex);

    /// Append a display equation as its own paragraph.
    Equation& addMath(const std::string& latex, EquationMode mode = EquationMode::Display);

    /// @{
    /// Append an image.  @p width and @p height are pixel dimensions; 0 means
    /// auto-detect from the file.
    Image& addImage(const std::string& path);
    Image& addImage(const std::string& path, int width, int height);
    Image& addImage(const char* path);
    Image& addImage(const std::wstring& path);
    Image& addImage(const std::filesystem::path& path);
    Image& addImage(const std::wstring& path, int w, int h);
    Image& addImage(const std::filesystem::path& path, int w, int h);
    /// @}

    /// Append an empty table with the given dimensions.
    Table& addTable(int rows, int cols);

    /// Append an empty bullet list.
    BulletList& addBulletList();

    /// Append an empty ordered (numbered) list.
    BulletList& addOrderedList();

    /// Run style inherited by content added afterwards.
    void setStyle(const RunStyle& style);

    /// The style set by setStyle() (mutable for chaining).
    RunStyle& getStyle();

    /// True when no content has been added.
    bool empty() const;

    /// Build the OOXML for this content (internal use).
    std::string toXml() const;

    /// @{
    /// Ownership transfer for template blocks; moved-from objects must not be
    /// reused.
    void append(Paragraph value);
    void append(Table value);
    void append(Image value);
    void append(BulletList value);
    void append(Equation value);
    /// @}

private:
    friend class Table;
    void     inheritStyle(const RunStyle* style);
    RunStyle effectiveStyle() const;
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

/// A footnote or endnote body.
class Note : public Content
{
    int id_;

public:
    /// @param id  Note ID, as used by Paragraph::addFootnoteRef().
    explicit Note(int id);
    /// The note's ID.
    int id() const;
};
} // namespace xword
