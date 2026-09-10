#pragma once

#include <string>
#include "Types.hpp"

namespace xword {

/// Run-level text formatting.  A default-constructed RunStyle applies no
/// extra formatting; chain setters to build a style inline:
///
///     RunStyle().bold().fontSize(14).color("FF0000")
///
class RunStyle {
public:
    RunStyle& bold(bool on = true)         { m_bold = on; m_boldState = on ? Toggle::On : Toggle::Off; return *this; }
    RunStyle& italic(bool on = true)       { m_italic = on; m_italicState = on ? Toggle::On : Toggle::Off; return *this; }
    RunStyle& underline(bool on = true)    { m_underline = on; m_underlineState = on ? Toggle::On : Toggle::Off; return *this; }
    RunStyle& fontSize(double pt)           { m_fontSize = pt; return *this; }
    RunStyle& color(const std::string& hex){ m_color = hex; return *this; }
    RunStyle& font(const std::string& name){ m_font = name; return *this; }
    RunStyle& eastAsiaFont(const std::string& name) { m_eastAsia = name; return *this; }
    RunStyle& strike(bool on = true) { m_strike = on ? Toggle::On : Toggle::Off; return *this; }
    RunStyle& superscript() { m_position = "superscript"; return *this; }
    RunStyle& subscript() { m_position = "subscript"; return *this; }
    RunStyle& baseline() { m_position = "baseline"; return *this; }
    RunStyle& highlight(const std::string& color) { m_highlight = color; return *this; }
    RunStyle& setStyleId(const std::string& id) { m_styleId = id; return *this; }
    RunStyle& inheritBold() { m_boldState = Toggle::Inherit; m_bold = false; return *this; }
    RunStyle& inheritItalic() { m_italicState = Toggle::Inherit; m_italic = false; return *this; }
    RunStyle& inheritUnderline() { m_underlineState = Toggle::Inherit; m_underline = false; return *this; }
    std::string toXml(const RunStyle* defaults = nullptr) const;

    bool bold()      const { return m_bold; }
    bool italic()    const { return m_italic; }
    bool underline() const { return m_underline; }
    double fontSize()  const { return m_fontSize; }
    const std::string& color() const { return m_color; }
    const std::string& font()  const { return m_font; }

    /// True if any formatting has been set.
    bool hasFormatting() const {
        return m_boldState != Toggle::Inherit || m_italicState != Toggle::Inherit || m_underlineState != Toggle::Inherit
            || m_strike != Toggle::Inherit || !m_position.empty() || !m_highlight.empty() || !m_eastAsia.empty() || !m_styleId.empty()
            || m_fontSize > 0 || !m_color.empty() || !m_font.empty();
    }

private:
    friend class Content;
    RunStyle withDefaults(const RunStyle& d) const;
    bool m_bold      = false;
    bool m_italic    = false;
    bool m_underline = false;
    double m_fontSize  = 0;
    std::string m_color;
    std::string m_font;
    Toggle m_boldState = Toggle::Inherit, m_italicState = Toggle::Inherit, m_underlineState = Toggle::Inherit, m_strike = Toggle::Inherit;
    std::string m_eastAsia, m_position, m_highlight, m_styleId;
};

} // namespace xword
