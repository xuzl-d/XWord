#pragma once

#include "Types.hpp"
#include <memory>
#include <string>
#include <filesystem>

namespace xword {

/// An image embedded in the document.
///
/// Images are added via Document::addImage().  The source file is read
/// at save time; missing files are silently skipped so the output docx
/// remains valid.
///
/// On Windows all path strings are interpreted as UTF-8 (the library
/// compiles with /utf-8).
class Image {
public:
    /// Construct from a UTF-8 string.
    Image(const std::string& filepath);

    /// Convenience overload for narrow string literals.
    Image(const char* filepath) : Image(std::string(filepath)) {}

    /// Construct from a filesystem path (portable).
    Image(const std::filesystem::path& filepath);

    /// Construct from a wide-character path (Windows).
    Image(const std::wstring& filepath);

    /// Convenience overload for wide string literals.
    Image(const wchar_t* filepath) : Image(std::wstring(filepath)) {}

    ~Image();
    Image(Image&&) noexcept;
    Image& operator=(Image&&) noexcept;

    /// @{
    /// Builder-style setters (chainable).

    /// Set explicit pixel dimensions.  0 = auto-detect from file.
    Image& setSize(int width, int height);

    /// Horizontal alignment of the image paragraph.
    Image& setAlignment(Alignment align);

    /// Caption text displayed below the image.
    Image& setCaption(const std::string& caption);
    Image& setCaption(const std::wstring& caption);
    /// @}

    /// @{
    /// Read accessors.

    const std::string& filepath()     const;
    int                width()        const;
    int                height()       const;
    Alignment          alignment()    const;
    bool               hasAlignment() const;
    const std::string& caption()      const;
    /// @}

    /// Relative path inside the docx (word/media/xxx).
    std::string mediaPath() const;

    /// Override the in-zip media filename (used for non-ASCII sources).
    void               setMediaName(const std::string& name);
    const std::string& mediaName() const;

    /// Relationship ID placeholder.
    std::string rId() const;
    void        setRId(const std::string& rid);

    /// Whether this image was skipped (source file missing).
    bool skipped() const;
    void setSkipped(bool v);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace xword
