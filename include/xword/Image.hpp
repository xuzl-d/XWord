#pragma once

#include <filesystem>
#include <memory>
#include <string>

#include "Types.hpp"

namespace xword
{

/// An image embedded in the document.
///
/// Images are added via Document::addImage().  The source file is read
/// at save time; missing files are silently skipped so the output docx
/// remains valid.
///
/// On Windows all path strings are interpreted as UTF-8 (the library
/// compiles with /utf-8).
class Image
{
public:
    /// Construct from a UTF-8 string.
    Image(const std::string& filepath);

    /// Convenience overload for narrow string literals.
    Image(const char* filepath);

    /// Construct from a filesystem path (portable).
    Image(const std::filesystem::path& filepath);

    /// Construct from a wide-character path (Windows).
    Image(const std::wstring& filepath);

    /// Convenience overload for wide string literals.
    Image(const wchar_t* filepath);

    /// @name Rule of five
    /// An image owns its placement options: movable, not copyable.
    /// @{
    ~Image();
    Image(Image&&) noexcept;
    Image& operator=(Image&&) noexcept;
    /// @}

    /// @{
    /// Builder-style setters (chainable).

    /// Set explicit pixel dimensions.  0 = auto-detect from file.
    Image& setSize(int width, int height);

    /// Horizontal alignment of the image paragraph.
    Image& setAlignment(Alignment align);

    /// Caption text displayed below the image.
    Image& setCaption(const std::string& caption);
    Image& setCaption(const std::wstring& caption);

    /// Bookmark the image so it can be referenced and numbered.
    Image& setBookmark(const TargetId& target);

    /// How the image interacts with the surrounding text.
    Image& setWrap(ImageWrap wrap);

    /// Offset of an anchored image, relative to the given frames.
    /// @param horizontal  Reference frame for @p x.
    /// @param vertical    Reference frame for @p y.
    Image& setPosition(Length x, Length y, PositionRelative horizontal = PositionRelative::Margin,
                       PositionRelative vertical = PositionRelative::Paragraph);

    /// Crop the image.  Each side is a fraction of the source size in [0, 1);
    /// the remaining area must be non-empty.
    Image& setCrop(double left, double top, double right, double bottom);

    /// Preserve the aspect ratio when the size is set (default on).
    Image& setKeepAspectRatio(bool on = true);

    /// Alternative text for accessibility and the hover tooltip.
    Image& setAltText(const std::string& description, const std::string& title = "");

    /// Raster fallback rendered by readers that cannot show SVG.
    Image& setSvgFallback(const std::string& path);

    /// Set the displayed size directly in EMU-bearing units.
    /// @throws std::invalid_argument if either dimension is not positive.
    Image& setDimensions(Length width, Length height);

    /// Build the internal image element (internal use).
    std::string toXml(bool inlineImage = false) const;
    /// @}

    /// @{
    /// Read accessors.

    /// Source path the image was constructed with.
    const std::string& filepath() const;

    /// Requested pixel width, or 0 for auto.
    int width() const;

    /// Requested pixel height, or 0 for auto.
    int height() const;

    /// Alignment set by setAlignment().
    Alignment alignment() const;

    /// Whether an alignment was set explicitly.
    bool hasAlignment() const;

    /// Caption set by setCaption().
    const std::string& caption() const;
    /// @}

    /// Relative path inside the docx (word/media/xxx).
    std::string mediaPath() const;

    /// @{
    /// Override the in-zip media filename (used for non-ASCII sources).
    void               setMediaName(const std::string& name);
    const std::string& mediaName() const;
    /// @}

    /// @{
    /// Relationship ID placeholder, resolved when the package is written.
    std::string rId() const;
    void        setRId(const std::string& rid);
    /// @}

    /// @{
    /// Whether this image was skipped (source file missing).
    bool skipped() const;
    void setSkipped(bool v);
    /// @}

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace xword
