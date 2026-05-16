#pragma once

#include "pugixml.hpp"
#include <string>
#include <vector>
#include <fstream>

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

class XmlStringWriter : public pugi::xml_writer {
public:
    std::string result;
    void write(const void* data, size_t size) override {
        result.append(static_cast<const char*>(data), size);
    }
};

// Escape XML special chars in text content
std::string xmlEscape(const std::string& text);

// Compute image dimensions in EMU from pixel size
struct ImageSize { int widthEmu; int heightEmu; };
ImageSize computeImageSize(const std::string& filepath, int desiredW, int desiredH);

// Get image content type based on extension
std::string imageContentType(const std::string& ext);

} // namespace internal
} // namespace xword
