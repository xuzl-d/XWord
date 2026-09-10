#pragma once
#include "Paragraph.hpp"
#include "Equation.hpp"
#include <memory>

namespace xword {
class Table;
class BulletList;
/// Ordered content owned by its parent. References remain valid until parent destruction.
class Content {
public:
    Content();
    virtual ~Content();
    Content(Content&&) noexcept;
    Content& operator=(Content&&) noexcept;
    Content(const Content&) = delete;
    Content& operator=(const Content&) = delete;
    Paragraph& addParagraph(const std::string& text = "");
    Paragraph& addParagraph(const std::wstring& text);
    Paragraph& addParagraph(const std::string& text,const RunStyle& style);
    Paragraph& addParagraph(const std::wstring& text,const RunStyle& style);
    Paragraph& addEquation(const std::string& latex);
    Paragraph& addEquation(const std::wstring& latex);
    Equation& addMath(const std::string& latex,EquationMode mode = EquationMode::Display);
    Image& addImage(const std::string& path);
    Image& addImage(const std::string& path,int width,int height);
    Image& addImage(const char* path) { return addImage(std::string(path)); }
    Image& addImage(const std::wstring& path) { return addImage(std::filesystem::path(path).u8string()); }
    Image& addImage(const std::filesystem::path& path) { return addImage(path.u8string()); }
    Image& addImage(const std::wstring& path,int w,int h) { return addImage(std::filesystem::path(path).u8string(),w,h); }
    Image& addImage(const std::filesystem::path& path,int w,int h) { return addImage(path.u8string(),w,h); }
    Table& addTable(int rows,int cols);
    BulletList& addBulletList();
    BulletList& addOrderedList();
    void setStyle(const RunStyle& style);
    RunStyle& getStyle();
    bool empty() const;
    std::string toXml() const;
    /// Ownership transfer for template blocks; moved-from objects must not be reused.
    void append(Paragraph value);
    void append(Table value);
    void append(Image value);
    void append(BulletList value);
    void append(Equation value);
private:
    friend class Table;
    void inheritStyle(const RunStyle* style);
    RunStyle effectiveStyle() const;
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};
class Note : public Content {
    int id_;
public:
    explicit Note(int id) : id_(id) {}
    int id() const { return id_; }
};
}
