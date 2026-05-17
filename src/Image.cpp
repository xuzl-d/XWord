#include "xword/Image.hpp"
#include <filesystem>

namespace xword {

struct Image::Impl {
    std::string m_filepath;
    int         m_width = 0;
    int         m_height = 0;
    Alignment   m_alignment = Alignment::Left;
    bool        m_hasAlignment = false;
    std::string m_rid;
    std::string m_mediaName;
    std::string m_caption;
    bool        m_skipped = false;
};

namespace {
std::string pathToUtf8(const std::filesystem::path& p) {
    return p.u8string();
}
} // namespace

Image::Image(const std::string& filepath)
    : m_impl(std::make_unique<Impl>())
{
    m_impl->m_filepath = filepath;
    namespace fs = std::filesystem;
    fs::path p = fs::u8path(filepath);
    m_impl->m_rid = "rId_img_" + p.stem().u8string();
}

Image::Image(const std::filesystem::path& filepath)
    : m_impl(std::make_unique<Impl>())
{
    m_impl->m_filepath = pathToUtf8(filepath);
    m_impl->m_rid = "rId_img_" + filepath.stem().u8string();
}

Image::Image(const std::wstring& filepath)
    : Image(std::filesystem::path(filepath))
{}

Image::~Image() = default;
Image::Image(Image&&) noexcept = default;
Image& Image::operator=(Image&&) noexcept = default;

Image& Image::setSize(int width, int height) {
    m_impl->m_width = width;
    m_impl->m_height = height;
    return *this;
}

Image& Image::setCaption(const std::string& caption) {
    m_impl->m_caption = caption;
    return *this;
}

Image& Image::setAlignment(Alignment align) {
    m_impl->m_alignment = align;
    m_impl->m_hasAlignment = true;
    return *this;
}

const std::string& Image::filepath()     const { return m_impl->m_filepath; }
int                Image::width()        const { return m_impl->m_width; }
int                Image::height()       const { return m_impl->m_height; }
Alignment          Image::alignment()    const { return m_impl->m_alignment; }
bool               Image::hasAlignment() const { return m_impl->m_hasAlignment; }
const std::string& Image::caption()      const { return m_impl->m_caption; }

std::string Image::mediaPath() const {
    if (!m_impl->m_mediaName.empty())
        return "media/" + m_impl->m_mediaName;
    namespace fs = std::filesystem;
    return "media/" + fs::u8path(m_impl->m_filepath).filename().u8string();
}

void               Image::setMediaName(const std::string& name) { m_impl->m_mediaName = name; }
const std::string& Image::mediaName()    const { return m_impl->m_mediaName; }
std::string        Image::rId()          const { return m_impl->m_rid; }
void               Image::setRId(const std::string& rid)       { m_impl->m_rid = rid; }
bool               Image::skipped()      const { return m_impl->m_skipped; }
void               Image::setSkipped(bool v)                    { m_impl->m_skipped = v; }

} // namespace xword
