#pragma once

#include <string>

namespace xword {

class RunStyle {
public:
    RunStyle& bold(bool on = true) { m_bold = on; return *this; }
    RunStyle& italic(bool on = true) { m_italic = on; return *this; }
    RunStyle& underline(bool on = true) { m_underline = on; return *this; }
    RunStyle& fontSize(int pt) { m_fontSize = pt; return *this; }
    RunStyle& color(const std::string& hex) { m_color = hex; return *this; }
    RunStyle& font(const std::string& name) { m_font = name; return *this; }

    bool bold() const { return m_bold; }
    bool italic() const { return m_italic; }
    bool underline() const { return m_underline; }
    int fontSize() const { return m_fontSize; }
    const std::string& color() const { return m_color; }
    const std::string& font() const { return m_font; }

    bool hasFormatting() const {
        return m_bold || m_italic || m_underline || m_fontSize > 0 || !m_color.empty() || !m_font.empty();
    }

private:
    bool m_bold = false;
    bool m_italic = false;
    bool m_underline = false;
    int m_fontSize = 0;
    std::string m_color;
    std::string m_font;
};

} // namespace xword
