#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <unordered_map>

namespace xword {
namespace internal {

class ZipWriter {
public:
    explicit ZipWriter(const std::string& filepath);
    ~ZipWriter();

    ZipWriter(const ZipWriter&) = delete;
    ZipWriter& operator=(const ZipWriter&) = delete;

    bool addEntry(const std::string& name, const std::string& data);
    bool addFileEntry(const std::string& name, const std::string& filepath);
    bool finalize();

private:
    struct Impl;
    Impl* m_impl;
    std::string m_filepath;
};

// Read all entries from a ZIP file. Returns empty map on failure.
std::unordered_map<std::string, std::string> readZip(const std::string& filepath);

// Escape XML special chars in text content
std::string xmlEscape(const std::string& text);

// Compute image dimensions in EMU from pixel size
// maxWidthEmu: if > 0, auto-scale image to fit within this width (for page-fit behavior)
struct ImageSize { int widthEmu; int heightEmu; };
ImageSize computeImageSize(const std::string& filepath, int desiredW, int desiredH, int maxWidthEmu = 0);

// Get image content type based on extension
std::string imageContentType(const std::string& ext);

} // namespace internal
} // namespace xword
