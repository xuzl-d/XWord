#pragma once

#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

namespace xword
{

// ── Enums ────────────────────────────────────────────────

/// Paragraph / heading alignment.
enum class Alignment
{
    Left,
    Center,
    Right,
    Justify
};

/// Table cell vertical alignment.
enum class VAlignment
{
    Top,
    Center,
    Bottom
};

/// Table border style.
enum class TableStyle
{
    None,
    Grid,
    Light
};

/// Page size presets.
enum class PageSize
{
    A4,
    Letter
};

/// Page orientation.
enum class Orientation
{
    Portrait,
    Landscape
};

/// List kind (bullet or numbered).
enum class ListType
{
    Bullet,
    Ordered
};

/// Heading numbering format.
enum class HeadingNumFormat
{
    Decimal, ///< 1, 1.1, 1.1.1
    Chapter, ///< 第 1 章, 1.1, 1.1.1
};

/// LaTeX equation rendering mode.
enum class EquationMode
{
    Inline, ///< Inline math: m:oMath
    Display ///< Display math: m:oMathPara
};

/// Caption numbering strategy for images and tables.
enum class CaptionNumStyle
{
    Sequential, ///< Global 1, 2, 3, …
    ByChapter,  ///< Per-chapter: 1-1, 1-2, 2-1, … (H1 triggers chapter increment)
};

/// East-Asian document grid mode (ST_DocGrid, ECMA-376 §17.18.16).
enum class DocGridType
{
    Default,       ///< No grid; lines take their natural height.
    Lines,         ///< Line grid only.
    LinesAndChars, ///< Line and character grid.
    SnapToChars,   ///< Character grid only.
};

/// Section break type (corresponds to ST_SectionMark in ECMA-376 §17.18.77).
enum class SectionBreakType
{
    NextPage,   ///< New section starts on the following page (default).
    Continuous, ///< New section starts on the same page.
    EvenPage,   ///< New section starts on the next even-numbered page.
    OddPage,    ///< New section starts on the next odd-numbered page.
    NextColumn, ///< New section starts in the next column.
};

// ── Value types ──────────────────────────────────────────

/// Explicit physical length. Legacy numeric APIs retain their documented units.
class Length
{
    double points_ = 0;
    explicit Length(double p);

public:
    /// Zero length.
    Length();

    /// @{
    /// Construct from a unit. The value must be finite and within ±100000.
    static Length pt(double n);
    static Length cm(double n);
    static Length mm(double n);
    static Length inch(double n);
    static Length twips(int n);
    /// @}

    /// Length in twentieths of a point, the unit OOXML uses for most metrics.
    int dxa() const;

    /// Length in English Metric Units, the unit OOXML uses for drawing sizes.
    int emu() const;
};

/// Tri-state on/off flag: unset formatting inherits from the enclosing style.
enum class Toggle
{
    Inherit,
    Off,
    On
};

/// Kind of break inserted by Paragraph::addBreak().
enum class BreakType
{
    Line,
    Page,
    Column
};

/// Which header/footer story a Section method refers to.
enum class HeaderFooterType
{
    Default,
    First,
    Even
};

/// Number format for page, footnote and list numbering.
enum class NumberFormat
{
    Decimal,
    UpperRoman,
    LowerRoman,
    UpperLetter,
    LowerLetter,
    Bullet
};

/// When footnote numbering restarts.
enum class NoteRestart
{
    Continuous,
    EachSection,
    EachPage
};

/// Where footnotes are placed on the page.
enum class FootnotePosition
{
    PageBottom,
    BeneathText
};

/// Where endnotes are collected.
enum class EndnotePosition
{
    DocumentEnd,
    SectionEnd
};

/// What part of a bookmark a cross-reference points at.
enum class ReferenceKind
{
    Text,   ///< The bookmarked text itself.
    Number, ///< The number of the numbered item (caption, heading, …).
    Page    ///< The page the bookmark falls on.
};

/// What a bookmark marks, used to validate cross-references.
enum class TargetKind
{
    Bookmark,
    Paragraph,
    Heading,
    Figure,
    Table
};

/// How a line-spacing value in ParagraphStyle is interpreted.
enum class LineRule
{
    Auto,   ///< Multiple of single spacing (w:line/240).
    Exact,  ///< Exactly this height.
    AtLeast ///< At least this height.
};

/// Tab stop alignment.
enum class TabAlignment
{
    Left,
    Center,
    Right,
    Decimal,
    Bar
};

/// Leader character drawn before a tab stop.
enum class TabLeader
{
    None,
    Dot,
    Hyphen,
    Underscore
};

/// How an image is positioned relative to the text flow.
enum class ImageWrap
{
    Inline,       ///< Flows with the text (wp:inline).
    Square,       ///< Text wraps around the image.
    TopBottom,    ///< Text stops above and resumes below.
    BehindText,   ///< Floating, behind the text.
    InFrontOfText ///< Floating, in front of the text.
};

/// Reference frame for an anchored image's offset.
enum class PositionRelative
{
    Page,
    Margin,
    Column,
    Paragraph
};

/// Bibliography source category.
enum class SourceType
{
    Book,
    JournalArticle,
    ConferenceProceedings,
    Report,
    InternetSite,
    Misc
};

/// Type of a custom document property.
enum class PropertyType
{
    String,
    Number,
    Boolean,
    Date
};

/// Reason a save operation failed.
enum class SaveError
{
    None,
    InvalidArgument,
    InvalidXml,
    InvalidReference,
    MissingResource,
    IoError
};

/// Identifies a bookmark target for cross-references and captions.
struct TargetId
{
    /// Bookmark name; must match [A-Za-z_][A-Za-z0-9_]{0,39}.
    std::string name;
    /// What the bookmark marks.
    TargetKind kind = TargetKind::Bookmark;

    /// @param n  Bookmark name.
    /// @param k  What the bookmark marks.
    explicit TargetId(const std::string& n = "", TargetKind k = TargetKind::Bookmark);
};

/// A single problem reported while saving.
struct Diagnostic
{
    SaveError   code = SaveError::None; ///< Failure category.
    std::string part;                   ///< Package part the problem relates to (may be empty).
    std::string message;                ///< Human-readable description.
};

/// Options controlling Document::saveDetailed().
struct SaveOptions
{
    /// Treat a missing image file as a failed save instead of a warning that
    /// drops the image.
    bool missingImagesAreErrors = false;
};

/// Outcome of Document::saveDetailed().
struct SaveResult
{
    bool                    success = false; ///< True when the document was written.
    Diagnostic              error;           ///< Set when @ref success is false.
    std::vector<Diagnostic> warnings;        ///< Non-fatal problems, e.g. skipped images.

    /// Same as reading @ref success, so the result can be tested directly.
    explicit operator bool() const;
};

/// Numbering options for footnotes or endnotes.
struct NoteOptions
{
    NumberFormat     format           = NumberFormat::Decimal;        ///< Number format.
    int              start            = 1;                            ///< First number.
    NoteRestart      restart          = NoteRestart::Continuous;      ///< When numbering restarts.
    FootnotePosition footnotePosition = FootnotePosition::PageBottom; ///< Footnote placement.
    EndnotePosition  endnotePosition  = EndnotePosition::DocumentEnd; ///< Endnote placement.
};

/// One author of a bibliography source.
struct Author
{
    std::string first, last, middle; ///< Personal name parts.
    std::string corporate;           ///< Organisation name; exclusive with the name parts.
};

/// A bibliography entry referenced by Paragraph::addCitation().
struct BibliographySource
{
    std::string         tag;   ///< Citation key used by addCitation(); must be unique.
    std::string         title; ///< Source title (required).
    SourceType          type = SourceType::Book; ///< Source category.
    std::vector<Author> authors;                 ///< Authors, or a single corporate author.
    std::string         year, month, day;        ///< Publication date parts.
    std::string         publisher, city;         ///< Publication venue.
    std::string         journal;                 ///< Journal name (JournalArticle).
    std::string         volume, issue, pages;    ///< Journal/volume details.
    std::string         doi;                     ///< Standard number / DOI.
    std::string         url;                     ///< Internet address.
    std::string         accessedYear, accessedMonth, accessedDay; ///< Access date parts.
    int                 language = 1033;                          ///< LCID, 1033 = en-US.
};

/// Built-in document properties written to docProps/core.xml.
struct DocumentProperties
{
    std::string title;          ///< dc:title.
    std::string subject;        ///< dc:subject.
    std::string creator;        ///< dc:creator (author).
    std::string keywords;       ///< cp:keywords.
    std::string description;    ///< dc:description.
    std::string lastModifiedBy; ///< cp:lastModifiedBy.
    std::string language;       ///< dc:language, e.g. "zh-CN".
};

/// A user-defined document property.
struct CustomProperty
{
    std::string  value;                       ///< Value as text; parsed per @ref type.
    PropertyType type = PropertyType::String; ///< How @ref value is interpreted.
};

/// Convert a NumberFormat to its OOXML w:numFmt name.
/// @throws std::invalid_argument for NumberFormat::Bullet, which has no
///         numbering-format name (bullets are expressed as a symbol).
std::string numberFormatName(NumberFormat n);

/// Convert an Alignment to its OOXML w:jc value.
std::string alignmentToString(Alignment a);

/// Convert a VAlignment to its OOXML w:vAlign value.
std::string vAlignmentToString(VAlignment v);

/// Convert a DocGridType to its OOXML w:docGrid/@w:type value.
const char* docGridTypeToString(DocGridType t);

/// Convert a SectionBreakType to its OOXML w:type value.
std::string sectionBreakTypeToString(SectionBreakType t);

/// Page width in twips for the given size and orientation.
int pageWidthDxa(PageSize s, Orientation o);

/// Page height in twips for the given size and orientation.
int pageHeightDxa(PageSize s, Orientation o);

/// Page margin dimensions (cm).
struct PageMargins
{
    double top    = 2.54; ///< Top margin in cm.
    double bottom = 2.54; ///< Bottom margin in cm.
    double left   = 2.54; ///< Left margin in cm.
    double right  = 2.54; ///< Right margin in cm.
};

/// Page layout descriptor used by Section::setPage().
struct Page
{
    PageSize    size        = PageSize::A4;          ///< Paper size preset.
    Orientation orientation = Orientation::Portrait; ///< Paper orientation.
    PageMargins margins;                             ///< Margins in cm.
    int         customWidth = 0, customHeight = 0;   ///< Twips; override @ref size when non-zero.
    Length      headerDistance = Length::pt(36);     ///< Distance from the page edge to the header.
    Length      footerDistance = Length::pt(36);     ///< Distance from the page edge to the footer.
    Length      gutter;                              ///< Extra binding margin.

    /// Set an explicit paper size in twips, overriding @ref size.
    /// @throws std::invalid_argument if either dimension is not positive.
    Page& setCustomSize(Length width, Length height);

    /// Distance from the page edge to the header.
    Page& setHeaderDistance(Length n);

    /// Distance from the page edge to the footer.
    Page& setFooterDistance(Length n);

    /// Extra binding margin added to the inner edge.
    Page& setGutter(Length n);

    /// Select a paper size preset.
    Page& setSize(PageSize s);

    /// Select portrait or landscape.
    Page& setOrientation(Orientation o);

    /// Set all four margins in cm.
    Page& setMargins(double top, double bottom, double left, double right);
};

/// Heading style definition applied through Document::setHeadingStyle().
struct HeadingStyle
{
    std::string font;                           ///< Font name (empty = default).
    int         fontSize = 0;                   ///< pt, 0 = auto (44/36/30/28/26/24 for H1–H6).
    bool        bold     = true;                ///< Render the heading in bold.
    bool        italic   = false;               ///< Render the heading in italic.
    std::string color;                          ///< Hex RRGGBB (empty = auto).
    double      lineSpacing  = 0;               ///< e.g. 1.5 for 1.5×, 0 = default.
    double      spaceBefore  = -1;              ///< pt, -1 = use default.
    double      spaceAfter   = -1;              ///< pt, -1 = use default.
    Alignment   alignment    = Alignment::Left; ///< Paragraph alignment.
    bool        hasAlignment = false;           ///< Whether @ref alignment was set explicitly.

    /// Set the font for both East-Asian and Latin text.
    HeadingStyle& setFont(const std::string& f);

    /// Set the font size in points.
    HeadingStyle& setFontSize(int pt);

    /// Set bold on or off.
    HeadingStyle& setBold(bool b = true);

    /// Set italic on or off.
    HeadingStyle& setItalic(bool i = true);

    /// Set the text colour as hex RRGGBB.
    HeadingStyle& setColor(const std::string& c);

    /// Set the line spacing multiplier.
    HeadingStyle& setLineSpacing(double ls);

    /// Set the space before the heading in points.
    HeadingStyle& setSpaceBefore(double sb);

    /// Set the space after the heading in points.
    HeadingStyle& setSpaceAfter(double sa);

    /// Set the paragraph alignment.
    HeadingStyle& setAlignment(Alignment a);
};

} // namespace xword
