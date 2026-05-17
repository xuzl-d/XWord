#include "xword/Equation.hpp"
#include "internal/ZipWriter.hpp"
#include <cctype>
#include <stdexcept>
#include <unordered_map>
#include <string>
#include <string_view>

namespace xword {
namespace {

// ---- LaTeX → Unicode math character mapping ----
static const std::unordered_map<std::string_view, std::string> kGreekLetters = {
    {"alpha",   "\xCE\xB1"}, {"beta",    "\xCE\xB2"}, {"gamma",   "\xCE\xB3"},
    {"delta",   "\xCE\xB4"}, {"epsilon", "\xCE\xB5"}, {"zeta",    "\xCE\xB6"},
    {"eta",     "\xCE\xB7"}, {"theta",   "\xCE\xB8"}, {"iota",    "\xCE\xB9"},
    {"kappa",   "\xCE\xBA"}, {"lambda",  "\xCE\xBB"}, {"mu",      "\xCE\xBC"},
    {"nu",      "\xCE\xBD"}, {"xi",      "\xCE\xBE"}, {"pi",      "\xCF\x80"},
    {"rho",     "\xCF\x81"}, {"sigma",   "\xCF\x83"}, {"tau",     "\xCF\x84"},
    {"upsilon", "\xCF\x85"}, {"phi",     "\xCF\x86"}, {"chi",     "\xCF\x87"},
    {"psi",     "\xCF\x88"}, {"omega",   "\xCF\x89"},
    // Uppercase
    {"Gamma",   "\xCE\x93"}, {"Delta",   "\xCE\x94"}, {"Theta",   "\xCE\x98"},
    {"Lambda",  "\xCE\x9B"}, {"Xi",      "\xCE\x9E"}, {"Pi",      "\xCE\xA0"},
    {"Sigma",   "\xCE\xA3"}, {"Phi",     "\xCE\xA6"}, {"Psi",     "\xCE\xA8"},
    {"Omega",   "\xCE\xA9"},
    // Symbols
    {"infty",   "\xE2\x88\x9E"}, {"pm",   "\xC2\xB1"},   {"mp",     "\xE2\x88\x93"},
    {"times",   "\xC3\x97"},     {"div",  "\xC3\xB7"},   {"cdot",   "\xE2\x8B\x85"},
    {"leq",     "\xE2\x89\xA4"}, {"geq",  "\xE2\x89\xA5"}, {"neq",   "\xE2\x89\xA0"},
    {"approx",  "\xE2\x89\x88"}, {"equiv","\xE2\x89\xA1"}, {"sim",   "\xE2\x88\xBC"},
    {"subset",  "\xE2\x8A\x82"}, {"supset","\xE2\x8A\x83"}, {"subseteq","\xE2\x8A\x86"},
    {"cup",     "\xE2\x88\xAA"}, {"cap",  "\xE2\x88\xA9"}, {"emptyset","\xE2\x88\x85"},
    {"forall",  "\xE2\x88\x80"}, {"exists","\xE2\x88\x83"}, {"nabla", "\xE2\x88\x87"},
    {"partial", "\xE2\x88\x82"}, {"int",  "\xE2\x88\xAB"},  {"sum",   "\xE2\x88\x91"},
    {"prod",    "\xE2\x88\x8F"},
    // Arrows
    {"to",      "\xE2\x86\x92"}, {"rightarrow","\xE2\x86\x92"},
    {"leftarrow","\xE2\x86\x90"},{"leftrightarrow","\xE2\x86\x94"},
    {"Rightarrow","\xE2\x87\x92"},{"Leftarrow","\xE2\x87\x90"},
    // Dots
    {"cdots",   "\xE2\x8B\xAF"}, {"ldots",   "\xE2\x80\xA6"},
    {"vdots",   "\xE2\x8B\xAE"}, {"ddots",   "\xE2\x8B\xB1"},
    // Spacing
    {"quad",    " "},        {"qquad",   "  "},
    // More symbols
    {"in",      "\xE2\x88\x88"}, {"notin",   "\xE2\x88\x89"},
    {"ni",      "\xE2\x88\x8B"}, {"mid",     "\xE2\x88\xA3"},
    {"parallel","\xE2\x88\xA5"}, {"perp",    "\xE2\x8A\xA5"},
    {"angle",   "\xE2\x88\xA0"}, {"triangle","\xE2\x96\xB3"},
    {"propto",  "\xE2\x88\x9D"}, {"cong",    "\xE2\x89\x85"},
    {"ast",     "*"},            {"star",    "\xE2\x8B\x86"},
    {"circ",    "\xE2\x88\x98"}, {"bullet",  "\xE2\x80\xA2"},
    {"prime",   "\xE2\x80\xB2"}, {"oplus",   "\xE2\x8A\x95"},
    {"otimes",  "\xE2\x8A\x97"}, {"odot",    "\xE2\x8A\x99"},
    {"wp",      "\xE2\x84\x98"}, {"aleph",   "\xE2\x84\xB5"},
    // Operators
    {"sin", "sin"}, {"cos", "cos"}, {"tan", "tan"}, {"cot", "cot"},
    {"sec", "sec"}, {"csc", "csc"}, {"log", "log"}, {"ln",  "ln"},
    {"lim", "lim"}, {"max", "max"}, {"min", "min"}, {"det", "det"},
    {"arg", "arg"}, {"ker", "ker"}, {"dim", "dim"}, {"hom", "hom"},
    {"gcd", "gcd"}, {"lg",  "lg"},  {"Pr",  "Pr"},
};

// Accent types
enum class Accent {
    Bar, Hat, Tilde, Vec, Dot, Ddot
};

static const std::unordered_map<std::string_view, Accent> kAccents = {
    {"bar", Accent::Bar}, {"hat", Accent::Hat}, {"tilde", Accent::Tilde},
    {"vec", Accent::Vec}, {"dot", Accent::Dot}, {"ddot", Accent::Ddot},
};

static std::string accentOMML(Accent a, const std::string& base) {
    std::string tag;
    switch (a) {
        case Accent::Bar:   tag = "bar"; break;
        case Accent::Hat:   tag = "hat"; break;
        case Accent::Tilde: tag = "tilde"; break;
        case Accent::Vec:   tag = "vec"; break;
        case Accent::Dot:   tag = "dot"; break;
        case Accent::Ddot:  tag = "ddot"; break;
    }
    return "<m:acc><m:accPr><m:chr m:val=\"" + tag + "\"/></m:accPr>"
           "<m:e>" + base + "</m:e></m:acc>";
}

// Forward declaration
static std::string parseLaTeX(const char*& p, const char* end, bool stopAtAmp = false);

static void skipSpace(const char*& p, const char* end) {
    while (p < end && (*p == ' ' || *p == '\t')) ++p;
}

// Parse plain text until a special char
static std::string parseText(const char*& p, const char* end) {
    std::string result;
    while (p < end && std::isalpha(static_cast<unsigned char>(*p))) {
        result += *p++;
    }
    return result;
}

// Read digits
static std::string parseDigits(const char*& p, const char* end) {
    std::string result;
    while (p < end && std::isdigit(static_cast<unsigned char>(*p))) {
        result += *p++;
    }
    return result;
}

// Wrap content in <m:r><m:t>...</m:t></m:r>
static std::string run(const std::string& text) {
    return "<m:r><m:t xml:space=\"preserve\">"
           + internal::xmlEscape(text) + "</m:t></m:r>";
}

// Make an OMML base expression (text run in argument context)
static std::string base(const std::string& xml) {
    return "<m:e>" + xml + "</m:e>";
}

// Build an m:d (delimiter) with optional left/right
static std::string delim(const std::string& left, const std::string& content,
                          const std::string& right) {
    // Map LaTeX bracket to OMML character
    auto boxChar = [](char c) -> std::string {
        switch (c) {
            case '(': return "("; case ')': return ")";
            case '[': return "["; case ']': return "]";
            case '{': return "{"; case '}': return "}";
            case '|': return "|";
            default:  return std::string(1, c);
        }
    };

    std::string lch = left.empty() ? "" : boxChar(left[0]);
    std::string rch = right.empty() ? "" : boxChar(right[0]);

    return "<m:d>"
           "<m:dPr>"
           "<m:begChr m:val=\"" + (lch.empty() ? "." : lch) + "\"/>"
           "<m:endChr m:val=\"" + (rch.empty() ? "." : rch) + "\"/>"
           "</m:dPr>"
           "<m:e>" + content + "</m:e>"
           "</m:d>";
}

// Parse contents of { ... }
static std::string parseGroup(const char*& p, const char* end) {
    if (p >= end || *p != '{') return "";
    ++p; // skip {
    std::string result = parseLaTeX(p, end);
    if (p < end && *p == '}') ++p;
    return result;
}

// Parse n-th root: \sqrt[optional degree]{radicand}
static std::string parseSqrt(const char*& p, const char* end) {
    std::string degree;
    if (p < end && *p == '[') {
        ++p;
        degree = parseLaTeX(p, end);
        if (p < end && *p == ']') ++p;
    }
    std::string radicand = parseGroup(p, end);

    if (degree.empty()) {
        return "<m:rad><m:radPr><m:degHide/></m:radPr>"
               "<m:e>" + radicand + "</m:e></m:rad>";
    }
    return "<m:rad>"
           "<m:radPr/></m:radPr>"
           "<m:deg>" + degree + "</m:deg>"
           "<m:e>" + radicand + "</m:e>"
           "</m:rad>";
}

// Parse fraction: \frac{num}{den}
static std::string parseFrac(const char*& p, const char* end) {
    std::string num = parseGroup(p, end);
    std::string den = parseGroup(p, end);
    return "<m:f><m:fPr><m:ctrlPr/></m:fPr>"
           "<m:num>" + num + "</m:num>"
           "<m:den>" + den + "</m:den></m:f>";
}

// Parse \leftX ... \rightY
static std::string parseLeftRight(const char*& p, const char* end) {
    // Already consumed "left" or "right"
    // We handle this at the caller level
    return "";
}

// Parse matrix: \begin{pmatrix} ... \end{pmatrix}
static std::string parseMatrix(const char*& p, const char* end) {
    std::string rowsXml;
    std::string curRow;

    auto parseCell = [&]() {
        return "<m:e>" + parseLaTeX(p, end, true) + "</m:e>";
    };

    while (p < end) {
        if (p + 14 < end && std::string_view(p, 14) == "\\end{pmatrix}") {
            if (!curRow.empty()) {
                rowsXml += "<m:mr>" + curRow + "</m:mr>";
            }
            p += 14;
            break;
        }
        if (p + 2 < end && std::string_view(p, 2) == "\\\\") {
            p += 2;
            rowsXml += "<m:mr>" + curRow + "</m:mr>";
            curRow.clear();
            continue;
        }
        if (p < end && *p == '&') {
            ++p;
            curRow += parseCell();
            continue;
        }
        // Regular content starts a new cell
        curRow += parseCell();
    }

    return "<m:m>"
           "<m:mPr><m:ctrlPr/></m:mPr>"
           + rowsXml +
           "</m:m>";
}

// Parse subscripts and superscripts: _ or ^ followed by {group} or single char/digit
static std::string parseScript(const char*& p, const char* end, const std::string& baseOMML) {
    skipSpace(p, end);

    std::string sub, sup;

    // Parse sub _
    if (p < end && *p == '_') {
        ++p;
        if (p < end && *p == '{') {
            sub = parseGroup(p, end);
        } else {
            // Single character or digit
            std::string digit = parseDigits(p, end);
            if (!digit.empty()) {
                sub = run(digit);
            } else if (p < end && (*p == '\\' || std::isalpha(static_cast<unsigned char>(*p)))) {
                std::string t;
                while (p < end && std::isalpha(static_cast<unsigned char>(*p))) t += *p++;
                sub = run(t);
            }
        }
    }

    // Parse sup ^
    if (p < end && *p == '^') {
        ++p;
        if (p < end && *p == '{') {
            sup = parseGroup(p, end);
        } else {
            // Only ' for primes
            if (p < end && *p == '\'') {
                ++p;
                sup = run("\xE2\x80\xB2"); // prime ′
            } else {
                std::string digit = parseDigits(p, end);
                if (!digit.empty()) {
                    sup = run(digit);
                } else if (p < end && (*p == '\\' || std::isalpha(static_cast<unsigned char>(*p)))) {
                    std::string t;
                    while (p < end && std::isalpha(static_cast<unsigned char>(*p))) t += *p++;
                    sup = run(t);
                }
            }
        }
    }

    if (sub.empty() && sup.empty()) return baseOMML;

    if (!sub.empty() && !sup.empty()) {
        // Both: need nested sPre/sSub
        return "<m:sSubSup>"
               "<m:e>" + baseOMML + "</m:e>"
               "<m:sub>" + sub + "</m:sub>"
               "<m:sup>" + sup + "</m:sup>"
               "</m:sSubSup>";
    }
    if (!sub.empty()) {
        return "<m:sSub>"
               "<m:e>" + baseOMML + "</m:e>"
               "<m:sub>" + sub + "</m:sub>"
               "</m:sSub>";
    }
    return "<m:sSup>"
           "<m:e>" + baseOMML + "</m:e>"
           "<m:sup>" + sup + "</m:sup>"
           "</m:sSup>";
}

// Parse \int, \sum, \prod with limits
static std::string parseOperatorBody(const char*& p, const char* end) {
    skipSpace(p, end);
    if (p >= end) return "";

    // Read until a top-level stopping point
    std::string body;
    int depth = 0;
    while (p < end) {
        char c = *p;
        if (c == '}' && depth == 0) break;
        if (c == '&' && depth == 0) break;
        if (c == '\\' && depth == 0) {
            // Check for \\ or \end
            if (p + 1 < end) {
                if (*(p+1) == '\\') break;
                if (*(p+1) == 'e' && p + 4 < end && std::string_view(p, 5) == "\\end{") break;
                if (*(p+1) == 'b' && p + 6 < end && std::string_view(p, 7) == "\\begin{") break;
            }
        }
        if ((c == '+' || c == '=') && depth == 0) break;
        if (c == '-' && depth == 0) {
            // Only break at minus if followed by space or non-digit (not a negative number)
            if (p + 1 < end && (*(p+1) == ' ' || std::isalpha(static_cast<unsigned char>(*(p+1))))) break;
        }
        if (c == '{') ++depth;
        if (c == '}') --depth;
        body += c;
        ++p;
    }

    // Parse the collected body text through the LaTeX parser
    const char* bodyPtr = body.c_str();
    return parseLaTeX(bodyPtr, bodyPtr + body.size());
}

static std::string parseNOperator(const char*& p, const char* end, const std::string& opChar) {
    std::string sub, sup;

    // Parse lower limit _
    if (p < end && *p == '_') {
        ++p;
        if (p < end && *p == '{') {
            sub = parseGroup(p, end);
        } else {
            std::string t;
            while (p < end && !std::isspace(static_cast<unsigned char>(*p))
                   && *p != '^' && *p != ' ' && *p != '{' && *p != '\\') t += *p++;
            sub = run(t);
        }
    }

    // Parse upper limit ^
    if (p < end && *p == '^') {
        ++p;
        if (p < end && *p == '{') {
            sup = parseGroup(p, end);
        } else {
            std::string t;
            while (p < end && !std::isspace(static_cast<unsigned char>(*p))
                   && *p != '_' && *p != ' ' && *p != '{' && *p != '\\') t += *p++;
            sup = run(t);
        }
    }

    // Parse the body expression (what the operator applies to)
    std::string body = parseOperatorBody(p, end);

    std::string baseXml = run(opChar);

    if (sub.empty() && sup.empty() && body.empty()) return baseXml;
    if (sub.empty() && sup.empty()) {
        // No limits — just body
        return baseXml + body;
    }

    if (opChar == "\xE2\x88\xAB") { // ∫ integral
        if (!sub.empty() && !sup.empty()) {
            return "<m:nary><m:naryPr><m:chr m:val=\"\xE2\x88\xAB\"/>"
                   "<m:limLoc m:val=\"subSup\"/>"
                   "<m:ctrlPr/></m:naryPr>"
                   "<m:sub>" + sub + "</m:sub>"
                   "<m:sup>" + sup + "</m:sup>"
                   "<m:e>" + body + "</m:e></m:nary>";
        }
        if (!sub.empty()) {
            return "<m:nary><m:naryPr><m:chr m:val=\"\xE2\x88\xAB\"/>"
                   "<m:limLoc m:val=\"undOvr\"/>"
                   "<m:ctrlPr/></m:naryPr>"
                   "<m:sub>" + sub + "</m:sub>"
                   "<m:e>" + body + "</m:e></m:nary>";
        }
        return baseXml + body;
    }

    // Sum/Product with limits
    return "<m:nary><m:naryPr><m:chr m:val=\"" + opChar + "\"/>"
           "<m:limLoc m:val=\"subSup\"/>"
           "<m:ctrlPr/></m:naryPr>"
           "<m:sub>" + sub + "</m:sub>"
           "<m:sup>" + sup + "</m:sup>"
           "<m:e>" + body + "</m:e></m:nary>";
}

// Main recursive descent parser
static std::string parseLaTeX(const char*& p, const char* end, bool stopAtAmp) {
    std::string result;

    while (p < end) {
        // Stop conditions
        if (stopAtAmp && (*p == '&' || (p + 1 < end && *p == '\\' && *(p+1) == '\\'))) {
            break;
        }
        if (*p == '}' || *p == '&' || (p + 1 < end && *p == '\\' && *(p+1) == '\\')) {
            if (stopAtAmp && (*p == '}' || *p == '&')) break;
            if (!stopAtAmp && *p == '}') break;
        }

        // Subscript
        if (*p == '_') {
            if (!result.empty()) {
                result = parseScript(p, end, result);
            } else {
                ++p;
            }
            continue;
        }
        // Superscript
        if (*p == '^') {
            if (!result.empty()) {
                result = parseScript(p, end, result);
            } else {
                ++p;
            }
            continue;
        }

        // Backslash commands
        if (*p == '\\') {
            ++p;
            if (p >= end) break;

            // Read command name
            const char* cmdStart = p;
            while (p < end && std::isalpha(static_cast<unsigned char>(*p))) ++p;
            std::string_view cmd(cmdStart, p - cmdStart);

            // Check special commands
            if (cmd == "frac") {
                std::string fracXml = parseFrac(p, end);
                result += fracXml;
                continue;
            }
            if (cmd == "sqrt") {
                result += parseSqrt(p, end);
                continue;
            }
            if (cmd == "sum" || cmd == "prod" || cmd == "int") {
                std::string opChar;
                if (cmd == "sum") opChar = "\xE2\x88\x91";
                else if (cmd == "prod") opChar = "\xE2\x88\x8F";
                else opChar = "\xE2\x88\xAB";
                result += parseNOperator(p, end, opChar);
                continue;
            }
            if (cmd == "begin") {
                // \begin{pmatrix}
                if (p < end && *p == '{') {
                    ++p;
                    const char* envStart = p;
                    while (p < end && *p != '}') ++p;
                    std::string_view env(envStart, p - envStart);
                    if (p < end) ++p; // skip }
                    if (env == "pmatrix" || env == "matrix" || env == "bmatrix") {
                        result += parseMatrix(p, end);
                    }
                }
                continue;
            }
            if (cmd == "end") {
                // \end{...} - skip to }
                while (p < end && *p != '}') ++p;
                if (p < end) ++p;
                continue;
            }
            if (cmd == "left" || cmd == "right") {
                // \left( ... \right)
                if (p < end) {
                    char delimChar = *p++;
                    if (delimChar == '\\') {
                        // special delimiter like \rangle
                        while (p < end && std::isalpha(static_cast<unsigned char>(*p))) ++p;
                    }
                    if (cmd == "left") {
                        // We'll accumulate content until \right
                        // For simplicity, just print the bracket as text
                        if (delimChar == '.') { /* empty delimiter, skip */ }
                        else {
                            std::string d(1, delimChar);
                            result += run(d);
                        }
                    }
                }
                continue;
            }
            // Check for accents
            auto ait = kAccents.find(cmd);
            if (ait != kAccents.end()) {
                std::string inner = parseGroup(p, end);
                result += accentOMML(ait->second, inner);
                continue;
            }
            // Check for Greek/symbols
            auto git = kGreekLetters.find(cmd);
            if (git != kGreekLetters.end()) {
                result += run(git->second);
                continue;
            }
            // Unknown command - treat as text
            result += run(std::string(cmd));
            continue;
        }

        // Plain characters
        char c = *p;
        if (std::isalpha(static_cast<unsigned char>(c)) || std::isdigit(static_cast<unsigned char>(c))) {
            std::string text;
            while (p < end && (std::isalpha(static_cast<unsigned char>(*p))
                             || std::isdigit(static_cast<unsigned char>(*p)))) {
                text += *p++;
            }
            result += run(text);
            continue;
        }

        // Other characters (operators, punctuation)
        if (c == '+' || c == '-' || c == '=' || c == '<' || c == '>'
            || c == '*' || c == '/' || c == ',' || c == '.' || c == ':'
            || c == '!' || c == '?' || c == ';' || c == '(' || c == ')'
            || c == '[' || c == ']' || c == '|' || c == '\'') {
            result += run(std::string(1, c));
            ++p;
            continue;
        }

        // Space
        if (c == ' ' || c == '~') {
            result += run(c == '~' ? "\xE2\x80\x89" : " "); // thin space for ~
            ++p;
            continue;
        }

        // Unknown character - skip
        ++p;
    }

    return result;
}

} // anonymous namespace

// ---- Equation class ----

struct Equation::Impl {
    std::string  m_latex;
    EquationMode m_mode = EquationMode::Inline;
};

Equation::Equation(const std::string& latex, EquationMode mode)
    : m_impl(std::make_unique<Impl>(Impl{latex, mode})) {}

Equation::~Equation() = default;
Equation::Equation(Equation&&) noexcept = default;
Equation& Equation::operator=(Equation&&) noexcept = default;

Equation& Equation::setMode(EquationMode mode) {
    m_impl->m_mode = mode;
    return *this;
}

const std::string& Equation::latex() const { return m_impl->m_latex; }
EquationMode       Equation::mode()  const { return m_impl->m_mode; }

std::string Equation::toXml() const {
    if (m_impl->m_latex.empty()) return "";

    const char* p = m_impl->m_latex.c_str();
    const char* end = p + m_impl->m_latex.size();
    std::string content = parseLaTeX(p, end);

    if (m_impl->m_mode == EquationMode::Display) {
        return "<m:oMathPara>"
               "<m:oMath>" + content + "</m:oMath>"
               "</m:oMathPara>";
    }
    return "<m:oMath>" + content + "</m:oMath>";
}

} // namespace xword
