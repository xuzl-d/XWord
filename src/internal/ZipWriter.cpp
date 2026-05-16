#include "xword/internal/ZipWriter.hpp"
#include "miniz.h"
#include <fstream>
#include <cstring>

namespace xword {
namespace internal {

// ---- ZipWriter implementation ----

struct ZipWriter::Impl {
    mz_zip_archive archive;
};

ZipWriter::ZipWriter(const std::string& filepath)
    : m_impl(new Impl), m_filepath(filepath)
{
    std::memset(&m_impl->archive, 0, sizeof(m_impl->archive));
    if (!mz_zip_writer_init_file(&m_impl->archive, filepath.c_str(), 0)) {
        throw std::runtime_error("Failed to initialize ZIP writer for: " + filepath);
    }
}

ZipWriter::~ZipWriter() {
    if (m_impl) {
        mz_zip_writer_end(&m_impl->archive);
        delete m_impl;
    }
}

bool ZipWriter::addEntry(const std::string& name, const std::string& data) {
    return mz_zip_writer_add_mem(&m_impl->archive, name.c_str(),
        data.data(), data.size(), MZ_DEFAULT_COMPRESSION);
}

bool ZipWriter::addFileEntry(const std::string& name, const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) return false;
    std::string data((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
    return addEntry(name, data);
}

bool ZipWriter::finalize() {
    return mz_zip_writer_finalize_archive(&m_impl->archive);
}

// ---- XML helpers ----

std::string xmlEscape(const std::string& text) {
    std::string result;
    result.reserve(text.size());
    for (char c : text) {
        switch (c) {
            case '&':  result += "&amp;"; break;
            case '<':  result += "&lt;"; break;
            case '>':  result += "&gt;"; break;
            case '"':  result += "&quot;"; break;
            case '\'': result += "&apos;"; break;
            default:   result += c; break;
        }
    }
    return result;
}

ImageSize computeImageSize(const std::string& filepath, int desiredW, int desiredH) {
    // DPI assumed to be 96 (standard screen DPI)
    // EMU (English Metric Unit): 1px = 9525 EMU (at 96 DPI)
    // 1 inch = 914400 EMU, at 96 DPI: 914400 / 96 = 9525 EMU per pixel
    constexpr int emuPerPixel = 9525;

    if (desiredW > 0 && desiredH > 0) {
        return { desiredW * emuPerPixel, desiredH * emuPerPixel };
    }

    // Read first few bytes to detect PNG or JPEG dimension
    // For now, use default size
    int w = desiredW > 0 ? desiredW : 400;
    int h = desiredH > 0 ? desiredH : 300;
    return { w * emuPerPixel, h * emuPerPixel };
}

std::string imageContentType(const std::string& ext) {
    if (ext == "png" || ext == "PNG") return "image/png";
    if (ext == "jpg" || ext == "jpeg" || ext == "JPG" || ext == "JPEG") return "image/jpeg";
    return "application/octet-stream";
}

} // namespace internal
} // namespace xword
