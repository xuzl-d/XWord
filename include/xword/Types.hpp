#pragma once

#include <string>
#include <cmath>
#include <stdexcept>
#include <vector>

namespace xword {

// ── Enums ────────────────────────────────────────────────

/// Paragraph / heading alignment.
enum class Alignment { Left, Center, Right, Justify };

/// Table cell vertical alignment.
enum class VAlignment { Top, Center, Bottom };

/// Table border style.
enum class TableStyle { None, Grid, Light };

/// Page size presets.
enum class PageSize { A4, Letter };

/// Page orientation.
enum class Orientation { Portrait, Landscape };

/// List kind (bullet or numbered).
enum class ListType { Bullet, Ordered };

/// Heading numbering format.
enum class HeadingNumFormat {
    Decimal,     ///< 1, 1.1, 1.1.1
    Chapter,     ///< 第 1 章, 1.1, 1.1.1
};

/// LaTeX equation rendering mode.
enum class EquationMode {
    Inline,      ///< Inline math: m:oMath
    Display      ///< Display math: m:oMathPara
};

/// Caption numbering strategy for images and tables.
enum class CaptionNumStyle {
    Sequential,  ///< Global 1, 2, 3, …
    ByChapter,   ///< Per-chapter: 1-1, 1-2, 2-1, … (H1 triggers chapter increment)
};

/// Section break type (corresponds to ST_SectionMark in ECMA-376 §17.18.77).
enum class SectionBreakType {
    NextPage,     ///< New section starts on the following page (default).
    Continuous,   ///< New section starts on the same page.
    EvenPage,     ///< New section starts on the next even-numbered page.
    OddPage,      ///< New section starts on the next odd-numbered page.
    NextColumn,   ///< New section starts in the next column.
};

// ── Value types ──────────────────────────────────────────

/// Explicit physical length. Legacy numeric APIs retain their documented units.
class Length {
    double points_ = 0;
    explicit Length(double p) : points_(p) {
        if (!std::isfinite(p) || std::abs(p) > 100000) throw std::invalid_argument("Invalid length");
    }
public:
    Length() = default;
    static Length pt(double n) { return Length(n); }
    static Length cm(double n) { return Length(n * 72 / 2.54); }
    static Length mm(double n) { return cm(n / 10); }
    static Length inch(double n) { return pt(n * 72); }
    static Length twips(int n) { return pt(n / 20.0); }
    int dxa() const { return static_cast<int>(std::lround(points_ * 20)); }
    int emu() const { return static_cast<int>(std::lround(points_ * 12700)); }
};

enum class Toggle { Inherit, Off, On };
enum class BreakType { Line, Page, Column };
enum class HeaderFooterType { Default, First, Even };
enum class NumberFormat { Decimal, UpperRoman, LowerRoman, UpperLetter, LowerLetter, Bullet };
enum class NoteRestart { Continuous, EachSection, EachPage };
enum class FootnotePosition { PageBottom, BeneathText };
enum class EndnotePosition { DocumentEnd, SectionEnd };
enum class ReferenceKind { Text, Number, Page };
enum class TargetKind { Bookmark, Paragraph, Heading, Figure, Table };
enum class LineRule { Auto, Exact, AtLeast };
enum class TabAlignment { Left, Center, Right, Decimal, Bar };
enum class TabLeader { None, Dot, Hyphen, Underscore };
enum class ImageWrap { Inline, Square, TopBottom, BehindText, InFrontOfText };
enum class PositionRelative { Page, Margin, Column, Paragraph };
enum class SourceType { Book, JournalArticle, ConferenceProceedings, Report, InternetSite, Misc };
enum class PropertyType { String, Number, Boolean, Date };
enum class SaveError { None, InvalidArgument, InvalidXml, InvalidReference, MissingResource, IoError };

struct TargetId {
    std::string name;
    TargetKind kind = TargetKind::Bookmark;
    explicit TargetId(const std::string& n = "", TargetKind k = TargetKind::Bookmark) : name(n), kind(k) {}
};
struct Diagnostic { SaveError code = SaveError::None; std::string part, message; };
struct SaveOptions { bool missingImagesAreErrors = false; };
struct SaveResult {
    bool success = false;
    Diagnostic error;
    std::vector<Diagnostic> warnings;
    explicit operator bool() const { return success; }
};
struct NoteOptions {
    NumberFormat format = NumberFormat::Decimal;
    int start = 1;
    NoteRestart restart = NoteRestart::Continuous;
    FootnotePosition footnotePosition = FootnotePosition::PageBottom;
    EndnotePosition endnotePosition = EndnotePosition::DocumentEnd;
};
struct Author { std::string first, last, middle, corporate; };
struct BibliographySource {
    std::string tag, title;
    SourceType type = SourceType::Book;
    std::vector<Author> authors;
    std::string year, month, day, publisher, city, journal, volume, issue, pages, doi, url;
    std::string accessedYear, accessedMonth, accessedDay;
    int language = 1033;
};
struct DocumentProperties {
    std::string title, subject, creator, keywords, description, lastModifiedBy, language;
};
struct CustomProperty { std::string value; PropertyType type = PropertyType::String; };

inline std::string numberFormatName(NumberFormat n) {
    static const char* names[] = {"decimal", "upperRoman", "lowerRoman", "upperLetter", "lowerLetter", "bullet"};
    return names[static_cast<int>(n)];
}

/// Page margin dimensions (cm).
struct PageMargins {
    double top    = 2.54;
    double bottom = 2.54;
    double left   = 2.54;
    double right  = 2.54;
};

/// Page layout descriptor.
struct Page {
    PageSize     size        = PageSize::A4;
    Orientation  orientation = Orientation::Portrait;
    PageMargins  margins;
    int customWidth = 0, customHeight = 0;
    Length headerDistance = Length::pt(36), footerDistance = Length::pt(36), gutter;

    Page& setCustomSize(Length width, Length height) {
        if (width.dxa() <= 0 || height.dxa() <= 0) throw std::invalid_argument("Page dimensions must be positive");
        customWidth = width.dxa(); customHeight = height.dxa(); return *this;
    }
    Page& setHeaderDistance(Length n) { headerDistance = n; return *this; }
    Page& setFooterDistance(Length n) { footerDistance = n; return *this; }
    Page& setGutter(Length n) { gutter = n; return *this; }

    Page& setSize(PageSize s)               { size = s; return *this; }
    Page& setOrientation(Orientation o)     { orientation = o; return *this; }
    Page& setMargins(double top, double bottom, double left, double right) {
        margins = PageMargins{top, bottom, left, right};
        return *this;
    }
};

/// Heading style definition.
struct HeadingStyle {
    std::string font;           ///< Font name (empty = default).
    int    fontSize     = 0;    ///< pt, 0 = auto (44/36/30/28/26/24 for H1–H6).
    bool   bold         = true;
    bool   italic       = false;
    std::string color;          ///< Hex RRGGBB (empty = auto).
    double lineSpacing  = 0;    ///< e.g. 1.5 for 1.5×, 0 = default.
    double spaceBefore  = -1;   ///< pt, -1 = use default.
    double spaceAfter   = -1;   ///< pt, -1 = use default.
    Alignment alignment = Alignment::Left;
    bool   hasAlignment = false;

    HeadingStyle& setFont(const std::string& f)    { font = f; return *this; }
    HeadingStyle& setFontSize(int pt)              { fontSize = pt; return *this; }
    HeadingStyle& setBold(bool b = true)           { bold = b; return *this; }
    HeadingStyle& setItalic(bool i = true)         { italic = i; return *this; }
    HeadingStyle& setColor(const std::string& c)   { color = c; return *this; }
    HeadingStyle& setLineSpacing(double ls)        { lineSpacing = ls; return *this; }
    HeadingStyle& setSpaceBefore(double sb)        { spaceBefore = sb; return *this; }
    HeadingStyle& setSpaceAfter(double sa)         { spaceAfter = sa; return *this; }
    HeadingStyle& setAlignment(Alignment a)        { alignment = a; hasAlignment = true; return *this; }
};

// ── Helpers ──────────────────────────────────────────────

/// Convert Alignment enum to OOXML string.
inline std::string alignmentToString(Alignment a) {
    switch (a) {
        case Alignment::Left:    return "left";
        case Alignment::Center:  return "center";
        case Alignment::Right:   return "right";
        case Alignment::Justify: return "both";
    }
    return "left";
}

/// Convert VAlignment enum to OOXML string.
inline std::string vAlignmentToString(VAlignment v) {
    switch (v) {
        case VAlignment::Top:    return "top";
        case VAlignment::Center: return "center";
        case VAlignment::Bottom: return "bottom";
    }
    return "top";
}

/// Convert SectionBreakType enum to OOXML string.
inline std::string sectionBreakTypeToString(SectionBreakType t) {
    switch (t) {
        case SectionBreakType::NextPage:   return "nextPage";
        case SectionBreakType::Continuous: return "continuous";
        case SectionBreakType::EvenPage:   return "evenPage";
        case SectionBreakType::OddPage:    return "oddPage";
        case SectionBreakType::NextColumn: return "nextColumn";
    }
    return "nextPage";
}

/// Page width in twips for given size and orientation.
inline int pageWidthDxa(PageSize s, Orientation o) {
    if (o == Orientation::Landscape)
        return s == PageSize::A4 ? 16838 : 15840;
    return s == PageSize::A4 ? 11906 : 12240;
}

/// Page height in twips for given size and orientation.
inline int pageHeightDxa(PageSize s, Orientation o) {
    if (o == Orientation::Landscape)
        return s == PageSize::A4 ? 11906 : 12240;
    return s == PageSize::A4 ? 16838 : 15840;
}

} // namespace xword
