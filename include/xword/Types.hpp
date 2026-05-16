#pragma once

#include <string>

namespace xword {

enum class Alignment {
    Left,
    Center,
    Right,
    Justify
};

enum class TableStyle {
    None,
    Grid,
    Light,
};

enum class PageSize {
    A4,
    Letter,
};

enum class Orientation {
    Portrait,
    Landscape,
};

enum class ListType {
    Bullet,
    Ordered,
};

enum class HeadingNumFormat {
    Decimal,     // 1, 1.1, 1.1.1
    Chapter,     // 第1章, 1.1, 1.1.1
};

struct PageMargins {
    double top = 2.54;     // cm
    double bottom = 2.54;  // cm
    double left = 2.54;    // cm
    double right = 2.54;   // cm
};

struct Page {
    PageSize size = PageSize::A4;
    Orientation orientation = Orientation::Portrait;
    PageMargins margins;

    Page& setSize(PageSize s) { size = s; return *this; }
    Page& setOrientation(Orientation o) { orientation = o; return *this; }
    Page& setMargins(double top, double bottom, double left, double right) {
        margins = PageMargins{top, bottom, left, right};
        return *this;
    }
};

struct HeadingStyle {
    std::string font;           // font name, empty = default
    int fontSize = 0;           // pt, 0 = auto (44/36/30/28/26/24 for H1-H6)
    bool bold = true;
    bool italic = false;
    std::string color;          // hex RRGGBB, empty = auto
    double lineSpacing = 0;     // e.g. 1.5 for 1.5 line spacing, 0 = default
    double spaceBefore = -1;    // pt, -1 = use default
    double spaceAfter = -1;     // pt, -1 = use default
    Alignment alignment = Alignment::Left;
    bool hasAlignment = false;

    HeadingStyle& setFont(const std::string& f)      { font = f; return *this; }
    HeadingStyle& setFontSize(int pt)                 { fontSize = pt; return *this; }
    HeadingStyle& setBold(bool b = true)              { bold = b; return *this; }
    HeadingStyle& setItalic(bool i = true)            { italic = i; return *this; }
    HeadingStyle& setColor(const std::string& c)      { color = c; return *this; }
    HeadingStyle& setLineSpacing(double ls)           { lineSpacing = ls; return *this; }
    HeadingStyle& setSpaceBefore(double sb)           { spaceBefore = sb; return *this; }
    HeadingStyle& setSpaceAfter(double sa)            { spaceAfter = sa; return *this; }
    HeadingStyle& setAlignment(Alignment a)           { alignment = a; hasAlignment = true; return *this; }
};

inline std::string alignmentToString(Alignment a) {
    switch (a) {
        case Alignment::Left:   return "left";
        case Alignment::Center: return "center";
        case Alignment::Right:  return "right";
        case Alignment::Justify:return "both";
    }
    return "left";
}

inline int pageWidthDxa(PageSize s, Orientation o) {
    if (o == Orientation::Landscape) {
        return s == PageSize::A4 ? 16838 : 15840;
    }
    return s == PageSize::A4 ? 11906 : 12240;
}

inline int pageHeightDxa(PageSize s, Orientation o) {
    if (o == Orientation::Landscape) {
        return s == PageSize::A4 ? 11906 : 12240;
    }
    return s == PageSize::A4 ? 16838 : 15840;
}

} // namespace xword
