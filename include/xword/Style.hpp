#pragma once
#include "Run.hpp"
#include <optional>

namespace xword {
struct Border {
    std::string style = "single", color = "auto";
    Length width = Length::pt(0.5);
    int space = 0;
};
struct TabStop { Length position; TabAlignment alignment = TabAlignment::Left; TabLeader leader = TabLeader::None; };
struct ParagraphStyle {
    std::string basedOn, next, shading;
    RunStyle run;
    std::optional<Alignment> alignment;
    std::optional<Length> leftIndent, rightIndent, firstLine, hanging, before, after;
    std::optional<double> firstLineChars;
    double lineSpacing = 0;
    LineRule lineRule = LineRule::Auto;
    Toggle pageBreakBefore = Toggle::Inherit, keepNext = Toggle::Inherit, keepLines = Toggle::Inherit, widowControl = Toggle::Inherit;
    std::vector<TabStop> tabs;
    std::optional<Border> border;
    std::string toXml() const;
};
struct TableStyleDefinition {
    std::string basedOn, shading;
    RunStyle run;
    std::optional<Border> border;
    Length cellMargin = Length::pt(5.4);
};
}
