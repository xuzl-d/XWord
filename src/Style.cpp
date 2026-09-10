#include "xword/Style.hpp"
#include "internal/ZipWriter.hpp"
#include <cmath>

namespace xword {
using internal::xmlEscape;
RunStyle RunStyle::withDefaults(const RunStyle& d) const {
    RunStyle r=*this;
    if(r.m_boldState==Toggle::Inherit){r.m_boldState=d.m_boldState;r.m_bold=d.m_bold;}
    if(r.m_italicState==Toggle::Inherit){r.m_italicState=d.m_italicState;r.m_italic=d.m_italic;}
    if(r.m_underlineState==Toggle::Inherit){r.m_underlineState=d.m_underlineState;r.m_underline=d.m_underline;}
    if(r.m_strike==Toggle::Inherit)r.m_strike=d.m_strike;
    if(r.m_fontSize<=0)r.m_fontSize=d.m_fontSize;
    if(r.m_color.empty())r.m_color=d.m_color; if(r.m_font.empty())r.m_font=d.m_font;
    if(r.m_eastAsia.empty())r.m_eastAsia=d.m_eastAsia; if(r.m_position.empty())r.m_position=d.m_position;
    if(r.m_highlight.empty())r.m_highlight=d.m_highlight; if(r.m_styleId.empty())r.m_styleId=d.m_styleId;
    return r;
}
std::string RunStyle::toXml(const RunStyle* defaults) const {
    RunStyle empty; const auto& d = defaults ? *defaults : empty;
    auto str = [](const std::string& a, const std::string& b) { return a.empty() ? b : a; };
    std::string x;
    auto val = [&](const char* tag, const std::string& s) { if (!s.empty()) x += "<w:" + std::string(tag) + " w:val=\"" + xmlEscape(s) + "\"/>"; };
    val("rStyle", str(m_styleId, d.m_styleId));
    auto font = str(m_font, d.m_font), ea = str(m_eastAsia, d.m_eastAsia);
    if (!font.empty() || !ea.empty()) {
        x += "<w:rFonts";
        if (!font.empty()) x += " w:ascii=\"" + xmlEscape(font) + "\" w:hAnsi=\"" + xmlEscape(font) + "\"";
        if (!ea.empty()) x += " w:eastAsia=\"" + xmlEscape(ea) + "\"";
        x += "/>";
    }
    auto toggle = [&](const char* tag, Toggle a, Toggle b) { auto v = a == Toggle::Inherit ? b : a; if (v != Toggle::Inherit) val(tag, v == Toggle::On ? "1" : "0"); };
    toggle("b", m_boldState, d.m_boldState); toggle("i", m_italicState, d.m_italicState); toggle("strike", m_strike, d.m_strike);
    val("color", str(m_color, d.m_color));
    double size = m_fontSize > 0 ? m_fontSize : d.m_fontSize;
    if (size > 0) { val("sz", std::to_string(static_cast<int>(std::lround(size*2)))); val("szCs", std::to_string(static_cast<int>(std::lround(size*2)))); }
    val("highlight", str(m_highlight, d.m_highlight));
    auto u = m_underlineState == Toggle::Inherit ? d.m_underlineState : m_underlineState;
    if (u != Toggle::Inherit) val("u", u == Toggle::On ? "single" : "none");
    val("vertAlign", str(m_position, d.m_position));
    return x.empty() ? "" : "<w:rPr>" + x + "</w:rPr>";
}
std::string ParagraphStyle::toXml() const {
    std::string x;
    auto toggle = [&](const char* tag, Toggle t) { if (t != Toggle::Inherit) x += "<w:" + std::string(tag) + " w:val=\"" + (t == Toggle::On ? "1" : "0") + "\"/>"; };
    toggle("keepNext",keepNext); toggle("keepLines",keepLines); toggle("pageBreakBefore",pageBreakBefore); toggle("widowControl",widowControl);
    if (border) {
        x += "<w:pBdr>";
        for (auto side : {"top","left","bottom","right"}) x += "<w:" + std::string(side) + " w:val=\"" + xmlEscape(border->style) + "\" w:sz=\"" + std::to_string(border->width.dxa()*8/20) + "\" w:space=\"" + std::to_string(border->space) + "\" w:color=\"" + xmlEscape(border->color) + "\"/>";
        x += "</w:pBdr>";
    }
    if (!shading.empty()) x += "<w:shd w:val=\"clear\" w:fill=\"" + xmlEscape(shading) + "\"/>";
    if (!tabs.empty()) {
        x += "<w:tabs>";
        const char* aligns[] = {"left","center","right","decimal","bar"}; const char* leaders[] = {"none","dot","hyphen","underscore"};
        for (const auto& t : tabs) x += "<w:tab w:val=\"" + std::string(aligns[static_cast<int>(t.alignment)]) + "\" w:leader=\"" + leaders[static_cast<int>(t.leader)] + "\" w:pos=\"" + std::to_string(t.position.dxa()) + "\"/>";
        x += "</w:tabs>";
    }
    if (before || after || lineSpacing > 0) {
        x += "<w:spacing";
        if (before) x += " w:before=\"" + std::to_string(before->dxa()) + "\"";
        if (after) x += " w:after=\"" + std::to_string(after->dxa()) + "\"";
        if (lineSpacing > 0) { const char* rules[] = {"auto","exact","atLeast"}; x += " w:line=\"" + std::to_string(static_cast<int>(std::lround(lineSpacing * (lineRule == LineRule::Auto ? 240 : 20)))) + "\" w:lineRule=\"" + rules[static_cast<int>(lineRule)] + "\""; }
        x += "/>";
    }
    if (leftIndent || rightIndent || firstLine || hanging || firstLineChars) {
        x += "<w:ind";
        if (leftIndent) x += " w:left=\""+std::to_string(leftIndent->dxa())+"\"";
        if (rightIndent) x += " w:right=\""+std::to_string(rightIndent->dxa())+"\"";
        if (firstLine) x += " w:firstLine=\""+std::to_string(firstLine->dxa())+"\"";
        if (hanging) x += " w:hanging=\""+std::to_string(hanging->dxa())+"\"";
        if (firstLineChars) x += " w:firstLineChars=\""+std::to_string(static_cast<int>(std::lround(*firstLineChars*100)))+"\"";
        x += "/>";
    }
    if (alignment) x += "<w:jc w:val=\"" + alignmentToString(*alignment) + "\"/>";
    return x;
}
}
