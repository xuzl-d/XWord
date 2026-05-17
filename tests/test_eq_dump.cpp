#include "xword/xword.hpp"
#include <cassert>
#include <iostream>
#include <string>

static void dump(const char* name, const std::string& latex, xword::EquationMode m) {
    xword::Equation eq(latex, m);
    std::cout << "==== " << name << " ====\n";
    std::cout << "latex: " << latex << "\n";
    std::cout << "omml : " << eq.toXml() << "\n\n";
}

int main() {
    using namespace xword;

    // Cases that used to glue scripts to the entire prefix
    dump("3y^2z",      "3y^2z",                       EquationMode::Inline);
    dump("x_i+y_i",    "x_i+y_i",                     EquationMode::Inline);
    dump("group^2",    "{a+b}^2",                     EquationMode::Inline);
    dump("sqrt[y]",    "\\sqrt[y]{t_{v}^{2}}",        EquationMode::Inline);
    dump("nested",     "\\sqrt{1+{\\sqrt[y]{t_{v}^{2}}}^2}", EquationMode::Inline);
    dump("complex1",   "f(x,y,z) = 3y^2z(3 + \\frac{ 7x + 5 }{1 + y^2})", EquationMode::Display);
    dump("complex2",
         "f(x,y,z) = 3y^2z(3+\\frac{7x+5}{\\sqrt{1+{\\sqrt[y]{t_{v}^{2}}}^2}})"
         "\\sum_{i=0 }^{i=n}(i^2+2y_{2\\sin(i)}^{i+y})",
         EquationMode::Display);

    // Sanity: assert ^2 on 3y^2z attaches to y only (not "3y")
    std::string xml = Equation("3y^2z", EquationMode::Inline).toXml();
    // The base inside sSup must be just `y`, never `3y`.
    assert(xml.find(">3y</m:t>") == std::string::npos);
    assert(xml.find("<m:sSup><m:e><m:r><m:t xml:space=\"preserve\">y</m:t></m:r></m:e>")
           != std::string::npos);

    // x_i+y_i: two distinct sSub elements, NOT one big sSub wrapping x+y
    std::string xml2 = Equation("x_i+y_i", EquationMode::Inline).toXml();
    size_t first = xml2.find("<m:sSub>");
    size_t second = xml2.find("<m:sSub>", first + 1);
    assert(first != std::string::npos && second != std::string::npos);

    // Group {a+b}^2: base of sSup should contain both a and b runs
    std::string xml3 = Equation("{a+b}^2", EquationMode::Inline).toXml();
    assert(xml3.find("<m:sSup>") != std::string::npos);
    assert(xml3.find("<m:t xml:space=\"preserve\">a</m:t>") != std::string::npos);
    assert(xml3.find("<m:t xml:space=\"preserve\">b</m:t>") != std::string::npos);

    // No redundant xmlns:m on m:oMath / m:oMathPara
    assert(xml.find("xmlns:m=") == std::string::npos);
    std::string xmlDisp = Equation("E=mc^2", EquationMode::Display).toXml();
    assert(xmlDisp.find("<m:oMathPara>") != std::string::npos);
    assert(xmlDisp.find("xmlns:m=") == std::string::npos);

    std::cout << "All Equation::toXml assertions passed.\n";
    return 0;
}
