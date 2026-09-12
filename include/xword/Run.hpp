#pragma once

#include <string>

#include "Types.hpp"

namespace xword
{

/// Run-level text formatting.  A default-constructed RunStyle applies no
/// extra formatting; chain setters to build a style inline:
///
///     RunStyle().bold().fontSize(14).color("FF0000")
///
/// Every setter returns `*this`, so calls can be chained.  Leaving a toggle
/// untouched keeps it inheriting from the enclosing paragraph or character
/// style; passing `false` explicitly turns it off.
class RunStyle
{
public:
    /// Bold on/off.  Inherits when never set.
    RunStyle& bold(bool on = true);

    /// Italic on/off.  Inherits when never set.
    RunStyle& italic(bool on = true);

    /// Single underline on/off.  Inherits when never set.
    RunStyle& underline(bool on = true);

    /// Font size in points.  0 keeps the inherited size.
    RunStyle& fontSize(double pt);

    /// Text colour as hex RRGGBB (e.g. "FF0000").  Empty keeps the inherited colour.
    RunStyle& color(const std::string& hex);

    /// Font name for Latin text (w:ascii and w:hAnsi).
    RunStyle& font(const std::string& name);

    /// Font name for East-Asian text (w:eastAsia).
    RunStyle& eastAsiaFont(const std::string& name);

    /// Strikethrough on/off.  Inherits when never set.
    RunStyle& strike(bool on = true);

    /// Raise the run above the baseline.
    RunStyle& superscript();

    /// Lower the run below the baseline.
    RunStyle& subscript();

    /// Reset the baseline position to normal.
    RunStyle& baseline();

    /// Highlight colour, using Word's named values (e.g. "yellow").
    RunStyle& highlight(const std::string& color);

    /// Apply a character style by style ID.
    RunStyle& setStyleId(const std::string& id);

    /// @{
    /// Clear an inherited toggle without forcing it off.
    RunStyle& inheritBold();
    RunStyle& inheritItalic();
    RunStyle& inheritUnderline();
    /// @}

    /// Build the `<w:rPr>` fragment for this style.
    /// @param defaults  Style to fall back to for values left unset.
    std::string toXml(const RunStyle* defaults = nullptr) const;

    /// @{
    /// Current values.  For the toggles `false` also means "inherited"; for
    /// the others an empty string or 0 means "inherited".
    bool               bold() const;
    bool               italic() const;
    bool               underline() const;
    double             fontSize() const;
    const std::string& color() const;
    const std::string& font() const;
    /// @}

    /// True if any formatting has been set.
    bool hasFormatting() const;

private:
    friend class Content;
    RunStyle    withDefaults(const RunStyle& d) const;
    bool        m_bold      = false;
    bool        m_italic    = false;
    bool        m_underline = false;
    double      m_fontSize  = 0;
    std::string m_color;
    std::string m_font;
    Toggle      m_boldState = Toggle::Inherit, m_italicState = Toggle::Inherit;
    Toggle      m_underlineState = Toggle::Inherit, m_strike = Toggle::Inherit;
    std::string m_eastAsia, m_position, m_highlight, m_styleId;
};

} // namespace xword
