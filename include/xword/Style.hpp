#pragma once

#include <optional>

#include "Run.hpp"

namespace xword
{

/// A border line, used for paragraphs, table and cells.
struct Border
{
    std::string style = "single";        ///< OOXML line style, e.g. "single", "dashed", "nil".
    std::string color = "auto";          ///< Hex RRGGBB, or "auto".
    Length      width = Length::pt(0.5); ///< Line width (rounded to eighths of a point).
    int         space = 0;               ///< Padding between border and text, in points.
};

/// One tab stop of a paragraph.
struct TabStop
{
    Length       position;                       ///< Offset from the left margin.
    TabAlignment alignment = TabAlignment::Left; ///< Alignment at this stop.
    TabLeader    leader    = TabLeader::None;    ///< Fill character before the stop.
};

/// A paragraph style, registered with Document::registerParagraphStyle().
///
/// Unset members inherit from the style named by @ref basedOn.
struct ParagraphStyle
{
    std::string basedOn; ///< Style ID to inherit from ("" = Normal).
    std::string next;    ///< Style applied to the following paragraph (w:next).
    std::string shading; ///< Background fill colour as hex RRGGBB.
    RunStyle    run;     ///< Run formatting for text in this style.

    std::optional<Alignment> alignment;      ///< Horizontal alignment.
    std::optional<Length>    leftIndent;     ///< Left indent.
    std::optional<Length>    rightIndent;    ///< Right indent.
    std::optional<Length>    firstLine;      ///< First-line indent (absolute).
    std::optional<Length>    hanging;        ///< Hanging indent.
    std::optional<Length>    before;         ///< Space before, in twips.
    std::optional<Length>    after;          ///< Space after, in twips.
    std::optional<double>    firstLineChars; ///< First-line indent in characters.

    double                lineSpacing     = 0;               ///< 0 = inherit; 1.0 = single spacing.
    LineRule              lineRule        = LineRule::Auto;  ///< How lineSpacing is interpreted.
    Toggle                pageBreakBefore = Toggle::Inherit; ///< Start on a new page.
    Toggle                keepNext        = Toggle::Inherit; ///< Keep with the next paragraph.
    Toggle                keepLines       = Toggle::Inherit; ///< Keep all lines on one page.
    Toggle                widowControl    = Toggle::Inherit; ///< Avoid single-line widows/orphans.
    std::vector<TabStop>  tabs;                              ///< Tab stops.
    std::optional<Border> border;                            ///< Paragraph border.

    /// Build the `<w:pPr>` fragment for this style (internal use).
    std::string toXml() const;
};

/// A table style, registered with Document::registerTableStyle().
struct TableStyleDefinition
{
    std::string           basedOn; ///< Style ID to inherit from.
    std::string           shading; ///< Background fill colour as hex RRGGBB.
    RunStyle              run;     ///< Run formatting for text in this style.
    std::optional<Border> border;  ///< Border applied to the table and its cells.
    Length                cellMargin = Length::pt(5.4); ///< Default cell padding.
};
} // namespace xword
