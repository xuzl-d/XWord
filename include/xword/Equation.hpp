#pragma once

#include "Types.hpp"
#include <memory>
#include <string>

namespace xword {

/// A LaTeX equation rendered as Office Math Markup Language (OMML).
///
/// Equations may be added inline inside a Paragraph via
/// Paragraph::addEquation(), or as a standalone display equation via
/// Document::addDisplayEquation().
class Equation {
public:
    /// @param latex  LaTeX expression string.
    /// @param mode   Inline (default) or Display.
    explicit Equation(const std::string& latex,
                      EquationMode mode = EquationMode::Inline);
    explicit Equation(const std::wstring& latex,
                      EquationMode mode = EquationMode::Inline);

    ~Equation();
    Equation(Equation&&) noexcept;
    Equation& operator=(Equation&&) noexcept;

    /// Change the rendering mode after construction.
    Equation& setMode(EquationMode mode);

    /// The original LaTeX source.
    const std::string& latex() const;

    /// Current rendering mode.
    EquationMode mode() const;

    /// Build OMML XML for this equation.
    std::string toXml() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace xword
