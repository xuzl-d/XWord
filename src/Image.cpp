#include "xword/Image.hpp"
#include <filesystem>
#include "internal/ZipWriter.hpp"
#include "internal/utf.hpp"

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
    TargetId target;
    ImageWrap wrap = ImageWrap::Inline;
    PositionRelative horizontal = PositionRelative::Margin, vertical = PositionRelative::Paragraph;
    Length x, y;
    int widthEmu = 0, heightEmu = 0;
    double crop[4] = {0,0,0,0};
    bool aspect = true;
    std::string alt, title, fallback;
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
    if (width < 0 || height < 0 || width > 100000 || height > 100000) throw std::invalid_argument("Invalid image dimensions");
    m_impl->widthEmu = m_impl->heightEmu = 0;
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

Image& Image::setCaption(const std::wstring& s) { return setCaption(internal::wstring_to_utf8(s)); }
Image& Image::setBookmark(const TargetId& t) { m_impl->target=t; return *this; }
Image& Image::setWrap(ImageWrap w) { m_impl->wrap=w; return *this; }
Image& Image::setPosition(Length x,Length y,PositionRelative h,PositionRelative v) { m_impl->x=x; m_impl->y=y; m_impl->horizontal=h; m_impl->vertical=v; return *this; }
Image& Image::setCrop(double l,double t,double r,double b) {
    for(double v : {l,t,r,b}) if(!std::isfinite(v)||v<0||v>=1) throw std::invalid_argument("Crop must be a fraction in [0,1)");
    if(l+r>=1||t+b>=1) throw std::invalid_argument("Crop removes entire image");
    m_impl->crop[0]=l; m_impl->crop[1]=t; m_impl->crop[2]=r; m_impl->crop[3]=b; return *this;
}
Image& Image::setKeepAspectRatio(bool on) { m_impl->aspect=on; return *this; }
Image& Image::setAltText(const std::string& d,const std::string& t) { m_impl->alt=d; m_impl->title=t; return *this; }
Image& Image::setSvgFallback(const std::string& s) { m_impl->fallback=s; return *this; }
Image& Image::setDimensions(Length w,Length h) { if(w.emu()<=0||h.emu()<=0) throw std::invalid_argument("Invalid image dimensions"); m_impl->widthEmu=w.emu(); m_impl->heightEmu=h.emu(); return *this; }
std::string Image::toXml(bool inlineImage) const {
    using internal::xmlEscape;
    auto attr=[](const char* n,const std::string& v) { return " "+std::string(n)+"=\""+xmlEscape(v)+"\""; };
    std::string x="<xw:image"+attr("path",filepath())+attr("caption",caption())+attr("target",m_impl->target.name)+attr("kind",std::to_string(static_cast<int>(m_impl->target.kind)))+attr("width",std::to_string(width()))+attr("height",std::to_string(height()));
    x+=attr("cx",std::to_string(m_impl->widthEmu))+attr("cy",std::to_string(m_impl->heightEmu))+attr("inline",inlineImage?"1":"0")+attr("wrap",std::to_string(static_cast<int>(m_impl->wrap)))+attr("align",alignmentToString(alignment()));
    x+=attr("x",std::to_string(m_impl->x.emu()))+attr("y",std::to_string(m_impl->y.emu()))+attr("horizontal",std::to_string(static_cast<int>(m_impl->horizontal)))+attr("vertical",std::to_string(static_cast<int>(m_impl->vertical)));
    x+=attr("alt",m_impl->alt)+attr("title",m_impl->title)+attr("fallback",m_impl->fallback)+attr("aspect",m_impl->aspect?"1":"0");
    const char* sides[]={"l","t","r","b"}; for(int i=0;i<4;++i) x+=attr(sides[i],std::to_string(static_cast<int>(m_impl->crop[i]*100000)));
    return x+"/>";
}
} // namespace xword
