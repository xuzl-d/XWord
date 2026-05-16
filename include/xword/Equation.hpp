#pragma once

#include <string>

namespace xword {

enum class EquationMode {
    Inline,   // m:oMath
    Display   // m:oMathPara
};

class Equation {
public:
    explicit Equation(const std::string& latex, EquationMode mode = EquationMode::Inline);

    Equation& setMode(EquationMode mode);

    const std::string& latex() const { return m_latex; }
    EquationMode mode() const { return m_mode; }

    std::string toXml() const;

private:
    std::string m_latex;
    EquationMode m_mode;
};

} // namespace xword
