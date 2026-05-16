#pragma once

#include "Types.hpp"
#include <string>

namespace xword {

class Image {
public:
    Image(const std::string& filepath);

    Image& setSize(int width, int height);
    Image& setAlignment(Alignment align);
    Image& setCaption(const std::string& caption);

    const std::string& filepath() const { return m_filepath; }
    int width() const { return m_width; }
    int height() const { return m_height; }
    Alignment alignment() const { return m_alignment; }
    bool hasAlignment() const { return m_hasAlignment; }
    const std::string& caption() const { return m_caption; }

    // Get relative path in the docx (word/media/xxx)
    std::string mediaPath() const;

    // Get the relationship ID placeholder
    std::string rId() const { return m_rid; }
    void setRId(const std::string& rid) { m_rid = rid; }

private:
    std::string m_filepath;
    int m_width = 0;
    int m_height = 0;
    Alignment m_alignment = Alignment::Left;
    bool m_hasAlignment = false;
    std::string m_rid;
    std::string m_caption;
};

} // namespace xword
