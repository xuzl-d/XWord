#include "xword/Image.hpp"
#include <filesystem>

namespace xword {

namespace {
// Centralised conversions: all inputs land in UTF-8 m_filepath.
std::string pathToUtf8(const std::filesystem::path& p) {
    return p.u8string();
}
} // namespace

Image::Image(const std::string& filepath)
    : m_filepath(filepath)
{
    namespace fs = std::filesystem;
    fs::path p = fs::u8path(filepath);
    m_rid = "rId_img_" + p.stem().u8string();
}

Image::Image(const std::filesystem::path& filepath)
    : m_filepath(pathToUtf8(filepath))
{
    m_rid = "rId_img_" + filepath.stem().u8string();
}

Image::Image(const std::wstring& filepath)
    : Image(std::filesystem::path(filepath))
{}

Image& Image::setSize(int width, int height) {
    m_width = width;
    m_height = height;
    return *this;
}

Image& Image::setCaption(const std::string& caption) {
    m_caption = caption;
    return *this;
}

Image& Image::setAlignment(Alignment align) {
    m_alignment = align;
    m_hasAlignment = true;
    return *this;
}

std::string Image::mediaPath() const {
    if (!m_mediaName.empty()) {
        return "media/" + m_mediaName;
    }
    namespace fs = std::filesystem;
    return "media/" + fs::u8path(m_filepath).filename().u8string();
}

} // namespace xword
