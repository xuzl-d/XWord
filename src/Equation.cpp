#include "xword/Equation.hpp"
#include <cctype>
#include <string>
#include <string_view>
#include <cstring>
#include <memory>
#include <unordered_map>
#include <vector>

namespace xword {
    namespace {

        // ---------- 简单的 XML 转义 ----------
        static std::string xmlEscape(const std::string& s) {
            std::string out;
            for (char c : s) {
                switch (c) {
                case '&': out += "&amp;"; break;
                case '<': out += "&lt;"; break;
                case '>': out += "&gt;"; break;
                case '\'': out += "&apos;"; break;
                case '"': out += "&quot;"; break;
                default:  out += c; break;
                }
            }
            return out;
        }

        // ---------- LaTeX → Unicode 映射表 ----------
        static const std::unordered_map<std::string_view, std::string> kGreekLetters = {
            {"alpha",   "\xCE\xB1"}, {"beta",    "\xCE\xB2"}, {"gamma",   "\xCE\xB3"},
            {"delta",   "\xCE\xB4"}, {"epsilon", "\xCE\xB5"}, {"zeta",    "\xCE\xB6"},
            {"eta",     "\xCE\xB7"}, {"theta",   "\xCE\xB8"}, {"iota",    "\xCE\xB9"},
            {"kappa",   "\xCE\xBA"}, {"lambda",  "\xCE\xBB"}, {"mu",      "\xCE\xBC"},
            {"nu",      "\xCE\xBD"}, {"xi",      "\xCE\xBE"}, {"pi",      "\xCF\x80"},
            {"rho",     "\xCF\x81"}, {"sigma",   "\xCF\x83"}, {"tau",     "\xCF\x84"},
            {"upsilon", "\xCF\x85"}, {"phi",     "\xCF\x86"}, {"chi",     "\xCF\x87"},
            {"psi",     "\xCF\x88"}, {"omega",   "\xCF\x89"},
            {"Gamma",   "\xCE\x93"}, {"Delta",   "\xCE\x94"}, {"Theta",   "\xCE\x98"},
            {"Lambda",  "\xCE\x9B"}, {"Xi",      "\xCE\x9E"}, {"Pi",      "\xCE\xA0"},
            {"Sigma",   "\xCE\xA3"}, {"Phi",     "\xCE\xA6"}, {"Psi",     "\xCE\xA8"},
            {"Omega",   "\xCE\xA9"},
            {"infty",   "\xE2\x88\x9E"}, {"pm",   "\xC2\xB1"},   {"mp",     "\xE2\x88\x93"},
            {"times",   "\xC3\x97"},     {"div",  "\xC3\xB7"},   {"cdot",   "\xE2\x8B\x85"},
            {"leq",     "\xE2\x89\xA4"}, {"geq",  "\xE2\x89\xA5"}, {"neq",   "\xE2\x89\xA0"},
            {"approx",  "\xE2\x89\x88"}, {"equiv","\xE2\x89\xA1"}, {"sim",   "\xE2\x88\xBC"},
            {"subset",  "\xE2\x8A\x82"}, {"supset","\xE2\x8A\x83"}, {"subseteq","\xE2\x8A\x86"},
            {"cup",     "\xE2\x88\xAA"}, {"cap",  "\xE2\x88\xA9"}, {"emptyset","\xE2\x88\x85"},
            {"forall",  "\xE2\x88\x80"}, {"exists","\xE2\x88\x83"}, {"nabla", "\xE2\x88\x87"},
            {"partial", "\xE2\x88\x82"}, {"int",  "\xE2\x88\xAB"},  {"sum",   "\xE2\x88\x91"},
            {"prod",    "\xE2\x88\x8F"},
            {"to",      "\xE2\x86\x92"}, {"rightarrow","\xE2\x86\x92"},
            {"leftarrow","\xE2\x86\x90"},{"leftrightarrow","\xE2\x86\x94"},
            {"Rightarrow","\xE2\x87\x92"},{"Leftarrow","\xE2\x87\x90"},
            {"cdots",   "\xE2\x8B\xAF"}, {"ldots",   "\xE2\x80\xA6"},
            {"vdots",   "\xE2\x8B\xAE"}, {"ddots",   "\xE2\x8B\xB1"},
            {"quad",    " "},        {"qquad",   "  "},
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
            {"sin", "sin"}, {"cos", "cos"}, {"tan", "tan"}, {"cot", "cot"},
            {"sec", "sec"}, {"csc", "csc"}, {"log", "log"}, {"ln",  "ln"},
            {"lim", "lim"}, {"max", "max"}, {"min", "min"}, {"det", "det"},
            {"arg", "arg"}, {"ker", "ker"}, {"dim", "dim"}, {"hom", "hom"},
            {"gcd", "gcd"}, {"lg",  "lg"},  {"Pr",  "Pr"},
        };

        // 重音类型
        enum class Accent { Bar, Hat, Tilde, Vec, Dot, Ddot };
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

        // 前向声明
        static std::string parseLaTeX(const char*& p, const char* end, bool stopAtAmp = false);
        static std::string parseGroup(const char*& p, const char* end);
        static std::string parseSubSupArg(const char*& p, const char* end);
        static std::string parseSqrt(const char*& p, const char* end);
        static std::string parseFrac(const char*& p, const char* end);
        static std::string parseMatrix(const char*& p, const char* end, const std::string& env);
        static std::string parseNOperator(const char*& p, const char* end, const std::string& opChar);
        static std::string parseScript(const char*& p, const char* end, const std::string& baseOMML);

        void skipSpace(const char*& p, const char* end) {
            while (p < end && (*p == ' ' || *p == '\t')) ++p;
        }

        std::string run(const std::string& text) {
            if (text.empty()) return "";
            return "<m:r><m:t xml:space=\"preserve\">" + xmlEscape(text) + "</m:t></m:r>";
        }

        // 解析 { ... } 分组，返回内部 OMML（不含 <m:e>）
        static std::string parseGroup(const char*& p, const char* end) {
            if (p >= end || *p != '{') return "";
            ++p;
            std::string result = parseLaTeX(p, end);
            if (p < end && *p == '}') ++p;
            return result;
        }

        // 解析上下标参数（{group} 或单个字符/命令），返回内部 OMML 运行序列
        // （不带 <m:e> 包裹 — 调用方按需放进 <m:sub>/<m:sup> 等 CT_OMathArg 内）
        static std::string parseSubSupArg(const char*& p, const char* end) {
            if (p >= end) return "";
            if (*p == '{') {
                return parseGroup(p, end);
            }
            if (*p == '\\') {
                ++p;
                const char* cmdStart = p;
                while (p < end && std::isalpha(static_cast<unsigned char>(*p))) ++p;
                std::string_view cmd(cmdStart, p - cmdStart);
                auto git = kGreekLetters.find(cmd);
                if (git != kGreekLetters.end()) return run(git->second);
                return run(std::string(cmd));
            }
            // 单字符或数字
            std::string t;
            t += *p++;
            return run(t);
        }

        // 解析 \sqrt 及 \sqrt[degree]{radicand}
        static std::string parseSqrt(const char*& p, const char* end) {
            std::string degree;
            if (p < end && *p == '[') {
                ++p;
                // 手动提取 [...] 区间内容，避免 parseLaTeX 跨过 ']' 继续读
                std::string degSrc;
                int depth = 0;
                while (p < end) {
                    if (*p == ']' && depth == 0) break;
                    if (*p == '[') ++depth;
                    else if (*p == ']') --depth;
                    degSrc += *p++;
                }
                if (p < end && *p == ']') ++p;
                const char* dp = degSrc.c_str();
                const char* dend = dp + degSrc.size();
                degree = parseLaTeX(dp, dend, false);
            }
            std::string radicand = parseGroup(p, end);
            std::string radXml = "<m:e>" + radicand + "</m:e>";
            if (degree.empty()) {
                return "<m:rad><m:radPr><m:degHide m:val=\"on\"/></m:radPr>" + radXml + "</m:rad>";
            }
            else {
                return "<m:rad><m:radPr/>"
                    "<m:deg>" + degree + "</m:deg>"
                    + radXml + "</m:rad>";
            }
        }

        // 解析 \frac{num}{den}
        static std::string parseFrac(const char*& p, const char* end) {
            std::string num = parseGroup(p, end);
            std::string den = parseGroup(p, end);
            return "<m:f><m:fPr><m:ctrlPr/></m:fPr>"
                "<m:num>" + num + "</m:num>"
                "<m:den>" + den + "</m:den></m:f>";
        }

        // 解析矩阵环境（支持 matrix / pmatrix / bmatrix）
        static std::string parseMatrix(const char*& p, const char* end, const std::string& env) {
            std::string endMarker = "\\end{" + env + "}";
            std::string rowsXml;

            while (p < end) {
                // 检查结束标记
                if (p + endMarker.size() <= end &&
                    std::string_view(p, endMarker.size()) == endMarker) {
                    p += endMarker.size();
                    break;
                }
                // 解析一行
                std::string rowXml;
                while (p < end) {
                    skipSpace(p, end);
                    if (p + endMarker.size() <= end &&
                        std::string_view(p, endMarker.size()) == endMarker) break;
                    if (p + 1 < end && *p == '\\' && *(p + 1) == '\\') {
                        p += 2;
                        break;
                    }
                    if (*p == '&') {
                        ++p;
                        rowXml += "<m:e/>";   // 空单元格
                        continue;
                    }
                    std::string cell = parseLaTeX(p, end, true);
                    rowXml += "<m:e>" + cell + "</m:e>";
                    skipSpace(p, end);
                    if (p < end && *p == '&') { ++p; continue; }
                    if (p + 1 < end && *p == '\\' && *(p + 1) == '\\') { p += 2; break; }
                    break;
                }
                rowsXml += "<m:mr>" + rowXml + "</m:mr>";
            }

            std::string matrix = "<m:m><m:mPr><m:ctrlPr/></m:mPr>" + rowsXml + "</m:m>";

            if (env == "pmatrix") {
                return "<m:d><m:dPr>"
                    "<m:begChr m:val=\"(\"/><m:endChr m:val=\")\"/>"
                    "</m:dPr><m:e>" + matrix + "</m:e></m:d>";
            }
            else if (env == "bmatrix") {
                return "<m:d><m:dPr>"
                    "<m:begChr m:val=\"[\"/><m:endChr m:val=\"]\"/>"
                    "</m:dPr><m:e>" + matrix + "</m:e></m:d>";
            }
            return matrix;   // 无括号矩阵
        }

        // 处理上下标 _ 和 ^ ，返回添加了脚本的完整 OMML
        static std::string parseScript(const char*& p, const char* end, const std::string& baseOMML) {
            skipSpace(p, end);
            std::string sub, sup;

            if (p < end && *p == '_') {
                ++p;
                sub = parseSubSupArg(p, end);
            }
            if (p < end && *p == '^') {
                ++p;
                sup = parseSubSupArg(p, end);
            }

            if (sub.empty() && sup.empty()) return baseOMML;

            if (!sub.empty() && !sup.empty()) {
                return "<m:sSubSup>"
                    "<m:e>" + baseOMML + "</m:e>"
                    "<m:sub>" + sub + "</m:sub>"
                    "<m:sup>" + sup + "</m:sup>"
                    "</m:sSubSup>";
            }
            else if (!sub.empty()) {
                return "<m:sSub>"
                    "<m:e>" + baseOMML + "</m:e>"
                    "<m:sub>" + sub + "</m:sub>"
                    "</m:sSub>";
            }
            else {
                return "<m:sSup>"
                    "<m:e>" + baseOMML + "</m:e>"
                    "<m:sup>" + sup + "</m:sup>"
                    "</m:sSup>";
            }
        }

        // 解析 \int \sum \prod 等运算符，含上下限及后续主体
        static std::string parseNOperator(const char*& p, const char* end, const std::string& opChar) {
            std::string sub, sup;

            auto parseLimitArg = [&]() -> std::string {
                if (p < end && *p == '{') {
                    return parseGroup(p, end);
                }
                std::string t;
                while (p < end && !std::isspace(static_cast<unsigned char>(*p))
                    && *p != '^' && *p != '_' && *p != ' ' && *p != '{' && *p != '\\') {
                    t += *p++;
                }
                return run(t);
                };

            if (p < end && *p == '_') { ++p; sub = parseLimitArg(); }
            if (p < end && *p == '^') { ++p; sup = parseLimitArg(); }

            // 收集后续主体内容，直到遇到 & 、 \\ 或闭合花括号
            std::string body;
            int braceDepth = 0;
            while (p < end) {
                char c = *p;
                if (c == '}' && braceDepth == 0) break;
                if (c == '&') break;
                if (c == '\\' && p + 1 < end && *(p + 1) == '\\') break;
                if (c == '{') ++braceDepth;
                else if (c == '}') --braceDepth;
                body += c;
                ++p;
            }
            const char* bp = body.c_str();
            std::string bodyOMML = parseLaTeX(bp, bp + body.size(), false);

            if (sub.empty() && sup.empty()) {
                return "<m:nary><m:naryPr><m:chr m:val=\"" + opChar + "\"/>"
                    "<m:ctrlPr/></m:naryPr><m:e>" + bodyOMML + "</m:e></m:nary>";
            }

            return "<m:nary><m:naryPr><m:chr m:val=\"" + opChar + "\"/>"
                "<m:limLoc m:val=\"subSup\"/>"
                "<m:ctrlPr/></m:naryPr>"
                "<m:sub>" + sub + "</m:sub>"
                "<m:sup>" + sup + "</m:sup>"
                "<m:e>" + bodyOMML + "</m:e></m:nary>";
        }

        // 主递归解析器
        static std::string parseLaTeX(const char*& p, const char* end, bool stopAtAmp) {
            // 用 parts 累积，每个元素是一个"原子"：单个字符、命令或一个分组。
            // 这样 ^/_ 只作用于上一个原子，而不是迄今为止的全部内容。
            std::vector<std::string> parts;

            while (p < end) {
                // 终止条件
                if (stopAtAmp) {
                    if (*p == '&') break;
                    if (p + 1 < end && *p == '\\' && *(p + 1) == '\\') break;
                }
                else {
                    if (*p == '}') break;   // 分组结束
                }

                // 独立的上/下标（需要前面有基）
                if (*p == '_' || *p == '^') {
                    if (!parts.empty()) {
                        std::string base = std::move(parts.back());
                        parts.pop_back();
                        parts.push_back(parseScript(p, end, base));
                    }
                    else {
                        ++p;   // 忽略孤立的 _ ^
                    }
                    continue;
                }

                // 显式分组：{ ... } 作为一个原子，便于 ^/_ 作用于整组
                if (*p == '{') {
                    std::string inner = parseGroup(p, end);
                    if (!inner.empty()) parts.push_back(inner);
                    continue;
                }

                // 反斜杠命令
                if (*p == '\\') {
                    ++p;
                    if (p >= end) break;
                    const char* cmdStart = p;
                    while (p < end && std::isalpha(static_cast<unsigned char>(*p))) ++p;
                    std::string_view cmd(cmdStart, p - cmdStart);

                    if (cmd == "frac") {
                        parts.push_back(parseFrac(p, end));
                        continue;
                    }
                    if (cmd == "sqrt") {
                        parts.push_back(parseSqrt(p, end));
                        continue;
                    }
                    if (cmd == "sum" || cmd == "prod" || cmd == "int") {
                        std::string opChar;
                        if (cmd == "sum") opChar = "\xE2\x88\x91";
                        else if (cmd == "prod") opChar = "\xE2\x88\x8F";
                        else opChar = "\xE2\x88\xAB";
                        parts.push_back(parseNOperator(p, end, opChar));
                        continue;
                    }
                    if (cmd == "begin") {
                        if (p < end && *p == '{') {
                            ++p;
                            const char* envStart = p;
                            while (p < end && *p != '}') ++p;
                            std::string_view env(envStart, p - envStart);
                            if (p < end) ++p;  // skip '}'
                            if (env == "matrix" || env == "pmatrix" || env == "bmatrix") {
                                parts.push_back(parseMatrix(p, end, std::string(env)));
                            }
                        }
                        continue;
                    }
                    if (cmd == "end") {
                        while (p < end && *p != '}') ++p;
                        if (p < end) ++p;
                        continue;
                    }
                    // \left \right 简化处理：只输出原括号，不伸缩
                    if (cmd == "left" || cmd == "right") {
                        if (p < end) {
                            char delimChar = *p++;
                            if (delimChar == '.') { /* 无分隔符 */ }
                            else parts.push_back(run(std::string(1, delimChar)));
                        }
                        continue;
                    }
                    // 重音符号
                    auto ait = kAccents.find(cmd);
                    if (ait != kAccents.end()) {
                        std::string inner = parseGroup(p, end);
                        parts.push_back(accentOMML(ait->second, inner));
                        continue;
                    }
                    // 希腊字母及其他符号
                    auto git = kGreekLetters.find(cmd);
                    if (git != kGreekLetters.end()) {
                        parts.push_back(run(git->second));
                        continue;
                    }
                    // 未知命令，原样输出名称
                    parts.push_back(run(std::string(cmd)));
                    continue;
                }

                // 每个字母作为独立原子（保证 a^2 只让 a 上标）
                if (std::isalpha(static_cast<unsigned char>(*p))) {
                    parts.push_back(run(std::string(1, *p)));
                    ++p;
                    continue;
                }
                // 连续数字合并为一个数字原子（12^2 → 12 的平方）
                if (std::isdigit(static_cast<unsigned char>(*p))) {
                    std::string num;
                    while (p < end && std::isdigit(static_cast<unsigned char>(*p))) {
                        num += *p++;
                    }
                    parts.push_back(run(num));
                    continue;
                }

                // 运算符和标点
                if (strchr("+-=<>*/,.:;!?()[]|'`", *p)) {
                    parts.push_back(run(std::string(1, *p)));
                    ++p;
                    continue;
                }

                // 空格（包括 ~ 作为细空格）
                if (*p == ' ' || *p == '~') {
                    parts.push_back(run(*p == '~' ? "\xE2\x80\x89" : " "));
                    ++p;
                    continue;
                }

                // 跳过其他无法识别的字符
                ++p;
            }

            std::string result;
            for (const auto& s : parts) result += s;
            return result;
        }

    } // anonymous namespace

    // ---------- Equation 类实现 ----------

    struct Equation::Impl {
        std::string  m_latex;
        EquationMode m_mode = EquationMode::Inline;
    };

    Equation::Equation(const std::string& latex, EquationMode mode)
        : m_impl(std::make_unique<Impl>(Impl{ latex, mode })) {
    }

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

        // m: 命名空间由父文档 / 页眉 / 页脚根元素声明，这里不重复声明
        if (m_impl->m_mode == EquationMode::Display) {
            return "<m:oMathPara><m:oMath>" + content + "</m:oMath></m:oMathPara>";
        }
        return "<m:oMath>" + content + "</m:oMath>";
    }

} // namespace xword