#include "xword/internal/ZipWriter.hpp"
#include "miniz.h"
#include <fstream>
#include <cstring>
#include <cctype>
#include <cstdint>
#include <filesystem>

namespace xword {
namespace internal {

// Open file with proper Unicode path handling on Windows
static std::ifstream openFile(const std::string& filepath) {
#ifdef _WIN32
    std::filesystem::path p = std::filesystem::u8path(filepath);
    return std::ifstream(p, std::ios::binary);
#else
    return std::ifstream(filepath, std::ios::binary);
#endif
}

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
    auto file = openFile(filepath);
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

ImageSize computeImageSize(const std::string& filepath, int desiredW, int desiredH, int maxWidthEmu) {
    constexpr int emuPerPixel = 9525;  // 1px = 9525 EMU at 96 DPI
    constexpr int emuPerMM = 36000;    // 1mm = 36000 EMU

    auto scaleToFit = [maxWidthEmu](int w, int h) -> ImageSize {
        if (maxWidthEmu > 0 && w > maxWidthEmu) {
            double ratio = static_cast<double>(maxWidthEmu) / w;
            return { maxWidthEmu, static_cast<int>(h * ratio) };
        }
        return { w, h };
    };

    if (desiredW > 0 && desiredH > 0) {
        return scaleToFit(desiredW * emuPerPixel, desiredH * emuPerPixel);
    }

    // Try to read actual dimensions from the file
    auto file = openFile(filepath);
    if (file) {
        // Check if EMF: first 4 bytes = EMR_HEADER type (1)
        unsigned char hdr[4];
        file.read(reinterpret_cast<char*>(hdr), 4);
        if (file && hdr[0] == 1 && hdr[1] == 0 && hdr[2] == 0 && hdr[3] == 0) {
            // EMF: read Frame dimensions from EMR_HEADER (offset 24: 4 int32 in 0.01mm)
            // This is what Word uses when you drag an EMF into a document - vector scaling
            file.seekg(24);
            int32_t fx0 = 0, fy0 = 0, fx1 = 0, fy1 = 0;
            file.read(reinterpret_cast<char*>(&fx0), 4);
            file.read(reinterpret_cast<char*>(&fy0), 4);
            file.read(reinterpret_cast<char*>(&fx1), 4);
            file.read(reinterpret_cast<char*>(&fy1), 4);
            if (file && fx1 > fx0 && fy1 > fy0) {
                // Frame is in 0.01mm units: 1 unit = 360 EMU
                int32_t wEmu = (fx1 - fx0) * 360;
                int32_t hEmu = (fy1 - fy0) * 360;
                return scaleToFit(wEmu, hEmu);
            }
            // Fallback: szlMillimeters at offset 80
            file.seekg(80);
            int32_t mmW = 0, mmH = 0;
            file.read(reinterpret_cast<char*>(&mmW), 4);
            file.read(reinterpret_cast<char*>(&mmH), 4);
            if (file && mmW > 0 && mmH > 0) {
                return scaleToFit(mmW * emuPerMM, mmH * emuPerMM);
            }
        }
        // PNG: IHDR at offset 16 contains width/height as 4-byte big-endian
        file.seekg(0);
        unsigned char sig[8];
        file.read(reinterpret_cast<char*>(sig), 8);
        if (file && sig[0] == 0x89 && sig[1] == 'P' && sig[2] == 'N' && sig[3] == 'G') {
            file.seekg(16);
            unsigned char wh[8];
            file.read(reinterpret_cast<char*>(wh), 8);
            if (file) {
                int pngW = (wh[0] << 24) | (wh[1] << 16) | (wh[2] << 8) | wh[3];
                int pngH = (wh[4] << 24) | (wh[5] << 16) | (wh[6] << 8) | wh[7];
                if (pngW > 0 && pngH > 0) {
                    return { pngW * emuPerPixel, pngH * emuPerPixel };
                }
            }
        }
    }

    int w = desiredW > 0 ? desiredW : 400;
    int h = desiredH > 0 ? desiredH : 300;
    return { w * emuPerPixel, h * emuPerPixel };
}

std::string imageContentType(const std::string& ext) {
    // Normalize to lowercase
    std::string e;
    for (char c : ext) e += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

    if (e == "png")  return "image/png";
    if (e == "jpg" || e == "jpeg") return "image/jpeg";
    if (e == "gif")  return "image/gif";
    if (e == "bmp")  return "image/bmp";
    if (e == "tiff" || e == "tif") return "image/tiff";
    if (e == "emf")  return "image/x-emf";
    if (e == "wmf")  return "image/x-wmf";
    if (e == "svg")  return "image/svg+xml";
    return "application/octet-stream";
}

} // namespace internal
} // namespace xword
