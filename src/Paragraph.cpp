#include "xword/Paragraph.hpp"
#include "xword/Equation.hpp"

namespace xword {

Paragraph& Paragraph::addRun(const std::string& text) {
    m_runs.push_back({text, RunStyle(), RunKind::Text});
    return *this;
}

Paragraph& Paragraph::addRun(const std::string& text, const RunStyle& style) {
    m_runs.push_back({text, style, RunKind::Text});
    return *this;
}

Paragraph& Paragraph::addEquation(const std::string& latex) {
    m_runs.push_back({latex, RunStyle(), RunKind::InlineEquation});
    return *this;
}

Paragraph& Paragraph::setAlignment(Alignment align) {
    m_alignment = align;
    m_hasAlignment = true;
    return *this;
}

Paragraph& Paragraph::setFirstLineIndent(int twips) {
    m_firstLineIndent = twips;
    return *this;
}

Paragraph& Paragraph::setFirstLineIndentChars(double chars, int fontSizePt) {
    m_firstLineIndent = static_cast<int>(chars * fontSizePt * 20);
    return *this;
}

Paragraph& Paragraph::setSpacingAfter(int twips) {
    m_spacingAfter = twips;
    return *this;
}

Paragraph& Paragraph::setSpacingBefore(int twips) {
    m_spacingBefore = twips;
    return *this;
}

std::string Paragraph::toXml() const {
    using namespace internal;

    std::string xml;
    xml += "<w:p>";

    // Paragraph properties
    xml += "<w:pPr>";
    if (m_hasAlignment) {
        xml += "<w:jc w:val=\"" + alignmentToString(m_alignment) + "\"/>";
    }
    if (m_firstLineIndent >= 0) {
        xml += "<w:ind w:firstLine=\"" + std::to_string(m_firstLineIndent) + "\"/>";
    }
    if (m_spacingAfter >= 0 || m_spacingBefore >= 0) {
        xml += "<w:spacing";
        if (m_spacingAfter >= 0)  xml += " w:after=\"" + std::to_string(m_spacingAfter) + "\"";
        if (m_spacingBefore >= 0) xml += " w:before=\"" + std::to_string(m_spacingBefore) + "\"";
        xml += "/>";
    }
    xml += "</w:pPr>";

    // Runs
    for (const auto& run : m_runs) {
        if (run.kind == RunKind::InlineEquation) {
            // Inline equation: m:oMath inside w:r
            std::string eqXml = Equation(run.content, EquationMode::Inline).toXml();
            xml += "<w:r>" + eqXml + "</w:r>";
        } else {
            xml += "<w:r>";
            if (run.style.hasFormatting()) {
                xml += "<w:rPr>";
                if (run.style.bold()) xml += "<w:b/>";
                if (run.style.italic()) xml += "<w:i/>";
                if (run.style.underline()) xml += "<w:u w:val=\"single\"/>";
                if (run.style.fontSize() > 0) {
                    xml += "<w:sz w:val=\"" + std::to_string(run.style.fontSize() * 2) + "\"/>";
                    xml += "<w:szCs w:val=\"" + std::to_string(run.style.fontSize() * 2) + "\"/>";
                }
                if (!run.style.color().empty()) {
                    xml += "<w:color w:val=\"" + run.style.color() + "\"/>";
                }
                if (!run.style.font().empty()) {
                    xml += "<w:rFonts w:ascii=\"" + run.style.font() + "\" w:hAnsi=\"" + run.style.font() + "\"/>";
                }
                xml += "</w:rPr>";
            }
            xml += "<w:t xml:space=\"preserve\">" + xmlEscape(run.content) + "</w:t>";
            xml += "</w:r>";
        }
    }

    xml += "</w:p>";
    return xml;
}

} // namespace xword
