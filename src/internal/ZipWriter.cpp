#include "internal/ZipWriter.hpp"
#include "miniz.h"
#include <fstream>
#include <cstring>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <cmath>
#include <sstream>
#include "pugixml.hpp"

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
    FILE* file = nullptr;
};

ZipWriter::ZipWriter(const std::string& filepath)
    : m_impl(new Impl), m_filepath(filepath)
{
    std::memset(&m_impl->archive, 0, sizeof(m_impl->archive));
#ifdef _WIN32
    m_impl->file = _wfopen(std::filesystem::u8path(filepath).c_str(), L"wb");
#else
    m_impl->file = fopen(filepath.c_str(), "wb");
#endif
    if (!m_impl->file || !mz_zip_writer_init_cfile(&m_impl->archive, m_impl->file, 0)) {
        if (m_impl->file) fclose(m_impl->file);
        delete m_impl; m_impl = nullptr;
        throw std::runtime_error("Failed to initialize ZIP writer for: " + filepath);
    }
}

ZipWriter::~ZipWriter() {
    if (m_impl) {
        mz_zip_writer_end(&m_impl->archive);
        if (m_impl->file) fclose(m_impl->file);
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
    return mz_zip_writer_finalize_archive(&m_impl->archive) && fflush(m_impl->file) == 0 && !ferror(m_impl->file);
}

// ---- XML helpers ----

void validateXmlCharacters(const std::string& text) {
    for(size_t i=0;i<text.size();) {
        unsigned char first=static_cast<unsigned char>(text[i++]); uint32_t value=first; int count=0;
        if(first>=0xc2&&first<=0xdf){value=first&31;count=1;}
        else if(first>=0xe0&&first<=0xef){value=first&15;count=2;}
        else if(first>=0xf0&&first<=0xf4){value=first&7;count=3;}
        else if(first>=0x80)throw std::invalid_argument("Invalid UTF-8 text");
        for(int k=0;k<count;++k) { if(i>=text.size())throw std::invalid_argument("Incomplete UTF-8 text"); unsigned char b=static_cast<unsigned char>(text[i++]); if((b&0xc0)!=0x80)throw std::invalid_argument("Invalid UTF-8 text"); value=(value<<6)|(b&63); }
        if((count==1&&value<0x80)||(count==2&&value<0x800)||(count==3&&value<0x10000)||value>0x10ffff||(value>=0xd800&&value<=0xdfff))throw std::invalid_argument("Invalid UTF-8 code point");
        if((value<0x20&&value!=9&&value!=10&&value!=13)||value==0xfffe||value==0xffff)throw std::invalid_argument("Character is not permitted in XML 1.0");
    }
}
std::string xmlEscape(const std::string& text) {
    validateXmlCharacters(text);
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
    constexpr double pixel = 9525;
    double width=400*pixel,height=300*pixel;
    auto file=openFile(filepath);
    if(file) {
        std::string bytes((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
        auto u8=[&](size_t pos)->uint32_t { return pos<bytes.size()?static_cast<unsigned char>(bytes[pos]):0; };
        auto le16=[&](size_t p){return u8(p)|(u8(p+1)<<8);};
        auto le32=[&](size_t p){return u8(p)|(u8(p+1)<<8)|(u8(p+2)<<16)|(u8(p+3)<<24);};
        auto be16=[&](size_t p){return (u8(p)<<8)|u8(p+1);};
        auto be32=[&](size_t p){return (u8(p)<<24)|(u8(p+1)<<16)|(u8(p+2)<<8)|u8(p+3);};
        if(bytes.size()>=24 && bytes.compare(0,8,"\x89PNG\r\n\x1a\n",8)==0) {width=be32(16)*pixel;height=be32(20)*pixel;}
        else if(bytes.size()>=88 && le32(0)==1 && le32(40)==0x464d4520) {
            width=(static_cast<double>(static_cast<int32_t>(le32(32)))-static_cast<int32_t>(le32(24)))*360;
            height=(static_cast<double>(static_cast<int32_t>(le32(36)))-static_cast<int32_t>(le32(28)))*360;
        } else if(bytes.size()>=22 && le32(0)==0x9ac6cdd7 && le16(14)>0) {
            width=(static_cast<int16_t>(le16(10))-static_cast<int16_t>(le16(6)))*914400.0/le16(14);
            height=(static_cast<int16_t>(le16(12))-static_cast<int16_t>(le16(8)))*914400.0/le16(14);
        } else if(bytes.size()>=10 && bytes.compare(0,3,"GIF")==0) {width=le16(6)*pixel;height=le16(8)*pixel;}
        else if(bytes.size()>=26 && bytes.compare(0,2,"BM")==0) {width=le32(18)*pixel;height=std::abs(static_cast<double>(static_cast<int32_t>(le32(22))))*pixel;}
        else if(bytes.size()>4 && u8(0)==0xff && u8(1)==0xd8) {
            size_t p=2;
            while(p+4<bytes.size()) {
                if(u8(p++)!=0xff)continue;
                while(p<bytes.size()&&u8(p)==0xff)++p;
                unsigned marker=u8(p++);
                if(marker==0xda||marker==0xd9)break;
                if(marker==0x01||(marker>=0xd0&&marker<=0xd7))continue;
                auto length=be16(p); if(length<2||p+length>bytes.size())break;
                if((marker>=0xc0&&marker<=0xc3)||(marker>=0xc5&&marker<=0xc7)||(marker>=0xc9&&marker<=0xcb)||(marker>=0xcd&&marker<=0xcf)) {height=be16(p+3)*pixel;width=be16(p+5)*pixel;break;}
                p+=length;
            }
        } else if(bytes.size()>8 && (bytes.compare(0,2,"II")==0||bytes.compare(0,2,"MM")==0)) {
            bool little=bytes[0]=='I'; auto v16=[&](size_t p){return little?le16(p):be16(p);}; auto v32=[&](size_t p){return little?le32(p):be32(p);};
            size_t p=v32(4); if(p+2<=bytes.size()) { unsigned count=v16(p); for(unsigned i=0;i<count&&p+2+(i+1)*12<=bytes.size();++i) {size_t e=p+2+i*12;unsigned tag=v16(e);if((tag==256||tag==257)&&v32(e+4)==1){double n=v16(e+2)==3?v16(e+8):v32(e+8);if(tag==256)width=n*pixel;else height=n*pixel;}} }
        } else {
            pugi::xml_document svg; if(svg.load_buffer(bytes.data(),bytes.size())&&std::string(svg.document_element().name())=="svg") {
                auto root=svg.document_element(); auto length=[&](const char* value)->double { std::string s=value; if(s.empty()||s.find('%')!=std::string::npos)return 0; size_t used=0; try {double n=std::stod(s,&used);auto unit=s.substr(used);if(unit=="cm")return n*360000;if(unit=="mm")return n*36000;if(unit=="in")return n*914400;if(unit=="pt")return n*12700;return n*pixel;}catch(...){return 0;} };
                double w=length(root.attribute("width").value()),h=length(root.attribute("height").value());
                if(w>0&&h>0){width=w;height=h;} else {std::istringstream view(root.attribute("viewBox").value());double x,y,vw,vh;if(view>>x>>y>>vw>>vh){width=vw*pixel;height=vh*pixel;}}
            }
        }
    }
    if(!std::isfinite(width)||!std::isfinite(height)||width<=0||height<=0)throw std::invalid_argument("Invalid intrinsic image dimensions: "+filepath);
    if(desiredW>0&&desiredH>0){width=desiredW*pixel;height=desiredH*pixel;}
    else if(desiredW>0){height*=desiredW*pixel/width;width=desiredW*pixel;}
    else if(desiredH>0){width*=desiredH*pixel/height;height=desiredH*pixel;}
    if(maxWidthEmu>0&&width>maxWidthEmu){height*=maxWidthEmu/width;width=maxWidthEmu;}
    if(width>2147483647.0||height>2147483647.0)throw std::invalid_argument("Image dimensions overflow");
    return {static_cast<int>(std::lround(width)),static_cast<int>(std::lround(height))};
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
