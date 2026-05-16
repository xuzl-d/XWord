#include "xword/Image.hpp"
#include <filesystem>

namespace xword {

Image::Image(const std::string& filepath)
    : m_filepath(filepath)
{
    // Extract filename for media path
    namespace fs = std::filesystem;
    fs::path p(filepath);
    std::string ext = p.extension().string();
    m_rid = "rId_img_" + p.stem().string();
}

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
    namespace fs = std::filesystem;
    return "media/" + fs::path(m_filepath).filename().string();
}

} // namespace xword
