// Out-of-line definitions for the value types and enum helpers declared in
// xword/Types.hpp.  Keeping them here (rather than inline in the header) keeps
// the public header free of implementation detail.
#include "xword/Types.hpp"

#include <cmath>

namespace xword {

// ── Length ───────────────────────────────────────────────

Length::Length() = default;

Length::Length(double p) : points_(p)
{
    if (!std::isfinite(p) || std::abs(p) > 100000) {
        throw std::invalid_argument("Invalid length");
    }
}

// These factories are also evaluated from other headers — Page's and
// Style.hpp's default member initialisers — but those are ordinary calls
// resolved when the consumer links the library, so no `inline` is needed.
Length Length::pt(double n)
{
    return Length(n);
}

Length Length::cm(double n)
{
    return Length(n * 72 / 2.54);
}

Length Length::mm(double n)
{
    return cm(n / 10);
}

Length Length::inch(double n)
{
    return pt(n * 72);
}

Length Length::twips(int n)
{
    return pt(n / 20.0);
}

int Length::dxa() const
{
    return static_cast<int>(std::lround(points_ * 20));
}

int Length::emu() const
{
    return static_cast<int>(std::lround(points_ * 12700));
}

// ── Small value types ────────────────────────────────────

TargetId::TargetId(const std::string& n, TargetKind k) : name(n), kind(k) {}

SaveResult::operator bool() const { return success; }

// ── Enum → OOXML name ────────────────────────────────────

std::string numberFormatName(NumberFormat n)
{
    switch (n) {
        case NumberFormat::Decimal:     return "decimal";
        case NumberFormat::UpperRoman:  return "upperRoman";
        case NumberFormat::LowerRoman:  return "lowerRoman";
        case NumberFormat::UpperLetter: return "upperLetter";
        case NumberFormat::LowerLetter: return "lowerLetter";
        case NumberFormat::Bullet:      return "bullet";
    }
    throw std::invalid_argument("Invalid number format");
}

std::string alignmentToString(Alignment a)
{
    switch (a) {
        case Alignment::Left:    return "left";
        case Alignment::Center:  return "center";
        case Alignment::Right:   return "right";
        case Alignment::Justify: return "both";
    }
    return "left";
}

std::string vAlignmentToString(VAlignment v)
{
    switch (v) {
        case VAlignment::Top:    return "top";
        case VAlignment::Center: return "center";
        case VAlignment::Bottom: return "bottom";
    }
    return "top";
}

const char* docGridTypeToString(DocGridType t)
{
    switch (t) {
        case DocGridType::Default:       return "default";
        case DocGridType::Lines:         return "lines";
        case DocGridType::LinesAndChars: return "linesAndChars";
        case DocGridType::SnapToChars:   return "snapToChars";
    }
    return "default";
}

std::string sectionBreakTypeToString(SectionBreakType t)
{
    switch (t) {
        case SectionBreakType::NextPage:   return "nextPage";
        case SectionBreakType::Continuous: return "continuous";
        case SectionBreakType::EvenPage:   return "evenPage";
        case SectionBreakType::OddPage:    return "oddPage";
        case SectionBreakType::NextColumn: return "nextColumn";
    }
    return "nextPage";
}

int pageWidthDxa(PageSize s, Orientation o)
{
    if (o == Orientation::Landscape)
        return s == PageSize::A4 ? 16838 : 15840;
    return s == PageSize::A4 ? 11906 : 12240;
}

int pageHeightDxa(PageSize s, Orientation o)
{
    if (o == Orientation::Landscape)
        return s == PageSize::A4 ? 11906 : 12240;
    return s == PageSize::A4 ? 16838 : 15840;
}

// ── Page ─────────────────────────────────────────────────

Page& Page::setCustomSize(Length width, Length height)
{
    if (width.dxa() <= 0 || height.dxa() <= 0) {
        throw std::invalid_argument("Page dimensions must be positive");
    }
    customWidth = width.dxa();
    customHeight = height.dxa();
    return *this;
}

Page& Page::setHeaderDistance(Length n)
{
    headerDistance = n;
    return *this;
}

Page& Page::setFooterDistance(Length n)
{
    footerDistance = n;
    return *this;
}

Page& Page::setGutter(Length n)
{
    gutter = n;
    return *this;
}

Page& Page::setSize(PageSize s)
{
    size = s;
    return *this;
}

Page& Page::setOrientation(Orientation o)
{
    orientation = o;
    return *this;
}

Page& Page::setMargins(double top, double bottom, double left, double right)
{
    margins = PageMargins{top, bottom, left, right};
    return *this;
}

// ── HeadingStyle ─────────────────────────────────────────

HeadingStyle& HeadingStyle::setFont(const std::string& f)
{
    font = f;
    return *this;
}

HeadingStyle& HeadingStyle::setFontSize(int pt)
{
    fontSize = pt;
    return *this;
}

HeadingStyle& HeadingStyle::setBold(bool b)
{
    bold = b;
    return *this;
}

HeadingStyle& HeadingStyle::setItalic(bool i)
{
    italic = i;
    return *this;
}

HeadingStyle& HeadingStyle::setColor(const std::string& c)
{
    color = c;
    return *this;
}

HeadingStyle& HeadingStyle::setLineSpacing(double ls)
{
    lineSpacing = ls;
    return *this;
}

HeadingStyle& HeadingStyle::setSpaceBefore(double sb)
{
    spaceBefore = sb;
    return *this;
}

HeadingStyle& HeadingStyle::setSpaceAfter(double sa)
{
    spaceAfter = sa;
    return *this;
}

HeadingStyle& HeadingStyle::setAlignment(Alignment a)
{
    alignment = a;
    hasAlignment = true;
    return *this;
}

} // namespace xword
