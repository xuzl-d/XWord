#pragma once

#include "Types.hpp"
#include <string>
#include <filesystem>

namespace xword {

class Image {
public:
    // Construct from a UTF-8 path string, an fs::path, or a wide-char path.
    // On Windows, std::string is assumed to be UTF-8 (the project compiles with /utf-8).
    Image(const std::string& filepath);
    Image(const char* filepath) : Image(std::string(filepath)) {}
    Image(const std::filesystem::path& filepath);
    Image(const std::wstring& filepath);
    Image(const wchar_t* filepath) : Image(std::wstring(filepath)) {}

    Image& setSize(int width, int height);
    Image& setAlignment(Alignment align);
    Image& setCaption(const std::string& caption);

    const std::string& filepath() const { return m_filepath; }
    int width() const { return m_width; }
    int height() const { return m_height; }
    Alignment alignment() const { return m_alignment; }
    bool hasAlignment() const { return m_hasAlignment; }
    const std::string& caption() const { return m_caption; }

    // Get relative path in the docx (word/media/xxx).
    // For non-ASCII source filenames this is overridden by Document with a
    // sanitised ASCII name (OPC part names must be valid URI segments).
    std::string mediaPath() const;
    void setMediaName(const std::string& name) { m_mediaName = name; }
    const std::string& mediaName() const { return m_mediaName; }

    // Get the relationship ID placeholder
    std::string rId() const { return m_rid; }
    void setRId(const std::string& rid) { m_rid = rid; }

    // Skipped images (file missing/unreadable) are dropped from the docx
    // entirely so the document remains valid. Set during Document::save().
    bool skipped() const { return m_skipped; }
    void setSkipped(bool v) { m_skipped = v; }

private:
    std::string m_filepath;
    int m_width = 0;
    int m_height = 0;
    Alignment m_alignment = Alignment::Left;
    bool m_hasAlignment = false;
    std::string m_rid;
    std::string m_mediaName;  // ASCII-only filename used inside the docx
    std::string m_caption;
    bool m_skipped = false;
};

} // namespace xword
