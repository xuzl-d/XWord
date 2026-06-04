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
            {"leq",     "\xE2\x89\xA4"}, {"le",   "\xE2\x89\xA4"},
            {"geq",     "\xE2\x89\xA5"}, {"ge",   "\xE2\x89\xA5"},
            {"neq",     "\xE2\x89\xA0"},
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

        // 带 <m:sty m:val="p"/> 的文本运行 — 非斜体（用于 \text{}）
        std::string runPlain(const std::string& text) {
            if (text.empty()) return "";
            return "<m:r><m:rPr><m:sty m:val=\"p\"/></m:rPr>"
                   "<m:t xml:space=\"preserve\">" + xmlEscape(text) + "</m:t></m:r>";
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
            else if (env == "Bmatrix") {
                return "<m:d><m:dPr>"
                    "<m:begChr m:val=\"{ \"/><m:endChr m:val=\"}\"/>"
                    "</m:dPr><m:e>" + matrix + "</m:e></m:d>";
            }
            else if (env == "vmatrix") {
                return "<m:d><m:dPr>"
                    "<m:begChr m:val=\"|\"/><m:endChr m:val=\"|\"/>"
                    "</m:dPr><m:e>" + matrix + "</m:e></m:d>";
            }
            else if (env == "Vmatrix") {
                return "<m:d><m:dPr>"
                    "<m:begChr m:val=\"\xE2\x80\x96\"/><m:endChr m:val=\"\xE2\x80\x96\"/>"
                    "</m:dPr><m:e>" + matrix + "</m:e></m:d>";
            }
            else if (env == "cases") {
                return "<m:d><m:dPr>"
                    "<m:begChr m:val=\"{ \"/>"
                    "</m:dPr><m:e>" + matrix + "</m:e></m:d>";
            }
            return matrix;   // 无括号矩阵（matrix）
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

                    if (cmd == "text" || cmd == "mathrm") {
                        // \text{...} / \mathrm{...} — plain (roman) text in math
                        if (p < end && *p == '{') {
                            ++p;
                            int braceDepth = 1;
                            std::string raw;
                            while (p < end && braceDepth > 0) {
                                if (*p == '{') ++braceDepth;
                                else if (*p == '}') { --braceDepth; if (braceDepth == 0) { ++p; break; } }
                                if (braceDepth > 0) raw += *p++;
                            }
                            if (!raw.empty()) parts.push_back(runPlain(raw));
                        }
                        continue;
                    }
                    if (cmd == "mathbf") {
                        // \mathbf{...} — bold text in math
                        if (p < end && *p == '{') {
                            ++p;
                            int braceDepth = 1;
                            std::string raw;
                            while (p < end && braceDepth > 0) {
                                if (*p == '{') ++braceDepth;
                                else if (*p == '}') { --braceDepth; if (braceDepth == 0) { ++p; break; } }
                                if (braceDepth > 0) raw += *p++;
                            }
                            if (!raw.empty()) {
                                parts.push_back("<m:r><m:rPr><m:sty m:val=\"b\"/></m:rPr>"
                                    "<m:t xml:space=\"preserve\">" + xmlEscape(raw) + "</m:t></m:r>");
                            }
                        }
                        continue;
                    }
                    if (cmd == "mathit") {
                        // \mathit{...} — italic text in math (same as default math)
                        if (p < end && *p == '{') {
                            ++p;
                            int braceDepth = 1;
                            std::string raw;
                            while (p < end && braceDepth > 0) {
                                if (*p == '{') ++braceDepth;
                                else if (*p == '}') { --braceDepth; if (braceDepth == 0) { ++p; break; } }
                                if (braceDepth > 0) raw += *p++;
                            }
                            if (!raw.empty()) {
                                parts.push_back("<m:r><m:rPr><m:sty m:val=\"i\"/></m:rPr>"
                                    "<m:t xml:space=\"preserve\">" + xmlEscape(raw) + "</m:t></m:r>");
                            }
                        }
                        continue;
                    }
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
                            if (env == "matrix"   || env == "pmatrix" || env == "bmatrix"
                                || env == "vmatrix" || env == "Vmatrix" || env == "Bmatrix"
                                || env == "cases") {
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
                    // \left DELIM ... \right DELIM — stretchy brackets
                    if (cmd == "left") {
                        // Read delimiter: may be escaped (\{ → {), plain char, or .
                        char lDelim = '.';
                        if (p < end) {
                            if (*p == '\\' && p + 1 < end) { ++p; lDelim = *p++; }  // \{ → {
                            else lDelim = *p++;
                        }
                        // Collect raw content until matching \right (handle nesting)
                        std::string raw;
                        int lrDepth = 0;
                        while (p < end) {
                            if (p + 6 <= end && std::string_view(p, 6) == "\\right") {
                                if (lrDepth == 0) { p += 6; break; }
                                --lrDepth;
                            }
                            if (p + 5 <= end && std::string_view(p, 5) == "\\left") ++lrDepth;
                            raw += *p++;
                        }
                        char rDelim = '.';
                        if (p < end) {
                            if (*p == '\\' && p + 1 < end) { ++p; rDelim = *p++; }  // \} → }
                            else rDelim = *p++;
                        }
                        const char* rp = raw.c_str();
                        std::string inner = parseLaTeX(rp, rp + raw.size());
                        // Build stretchy bracket m:d. Note: Word defaults
                        // missing begChr/endChr to the matching pair of the
                        // present side. Emit empty m:val="" to suppress.
                        std::string dPr = "<m:dPr>";
                        dPr += "<m:begChr m:val=\"";
                        if (lDelim != '.') dPr += (lDelim == '{' ? "{ " : std::string(1, lDelim));
                        dPr += "\"/>";
                        dPr += "<m:endChr m:val=\"";
                        if (rDelim != '.') dPr += (rDelim == '}' ? "}" : std::string(1, rDelim));
                        dPr += "\"/>";
                        dPr += "</m:dPr>";
                        parts.push_back("<m:d>" + dPr + "<m:e>" + inner + "</m:e></m:d>");
                        continue;
                    }
                    if (cmd == "right") {
                        // orphan \right (shouldn't happen with valid LaTeX) — skip
                        if (p < end) ++p;
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

                // UTF-8 multibyte character (e.g. Chinese/Japanese/Korean)
                if (static_cast<unsigned char>(*p) >= 0x80) {
                    const char* start = p;
                    // Leading byte determines sequence length
                    int len = 1;
                    unsigned char c = static_cast<unsigned char>(*p);
                    if ((c & 0xE0) == 0xC0) len = 2;
                    else if ((c & 0xF0) == 0xE0) len = 3;
                    else if ((c & 0xF8) == 0xF0) len = 4;
                    while (len-- > 0 && p < end) ++p;
                    parts.push_back(run(std::string(start, p - start)));
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
        RunStyle     m_style;
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

    Equation& Equation::setStyle(const RunStyle& style) {
        m_impl->m_style = style;
        return *this;
    }

    const std::string& Equation::latex() const { return m_impl->m_latex; }
    EquationMode       Equation::mode()  const { return m_impl->m_mode; }

    std::string Equation::toXml() const {
        if (m_impl->m_latex.empty()) return "";

        const char* p = m_impl->m_latex.c_str();
        const char* end = p + m_impl->m_latex.size();
        std::string content = parseLaTeX(p, end);

        // Math runs take their style from <w:rPr> (e.g. <w:sz>, <w:color>),
        // NOT from <m:rPr> (there are no <m:sz>/<m:color> in OMML). Inject a
        // <w:rPr> block into every <m:r>, placed AFTER any existing <m:rPr>
        // (e.g. sty="p" from \text{}) — Word requires m:rPr first, then w:rPr.
        const auto& s = m_impl->m_style;
        bool hasStyle = s.hasFormatting();
        if (hasStyle) {
            std::string wRPr = "<w:rPr>";
            if (s.fontSize() > 0) {
                int halfPt = static_cast<int>(s.fontSize() * 2);
                wRPr += "<w:sz w:val=\"" + std::to_string(halfPt) + "\"/>"
                        "<w:szCs w:val=\"" + std::to_string(halfPt) + "\"/>";
            }
            if (s.bold())      wRPr += "<w:b/>";
            if (s.italic())    wRPr += "<w:i/>";
            if (s.underline()) wRPr += "<w:u w:val=\"single\"/>";
            if (!s.color().empty())
                wRPr += "<w:color w:val=\"" + s.color() + "\"/>";
            if (!s.font().empty())
                wRPr += "<w:rFonts w:ascii=\"" + s.font() + "\" w:hAnsi=\"" + s.font() + "\"/>";
            wRPr += "</w:rPr>";

            std::string result;
            size_t pos = 0;
            while (true) {
                size_t found = content.find("<m:r>", pos);
                if (found == std::string::npos) {
                    result += content.substr(pos);
                    break;
                }
                // Copy everything up to and including "<m:r>"
                result += content.substr(pos, found - pos + 5);
                size_t afterTag = found + 5; // skip "<m:r>"
                if (content.compare(afterTag, 7, "<m:rPr>") == 0) {
                    // Existing m:rPr — copy through "</m:rPr>" then insert w:rPr
                    size_t mRPrEnd = content.find("</m:rPr>", afterTag);
                    if (mRPrEnd != std::string::npos) {
                        size_t copyEnd = mRPrEnd + 8; // past "</m:rPr>"
                        result += content.substr(afterTag, copyEnd - afterTag);
                        result += wRPr;
                        pos = copyEnd;
                    } else {
                        pos = afterTag;
                    }
                } else {
                    // No m:rPr — w:rPr comes immediately after <m:r>
                    result += wRPr;
                    pos = afterTag;
                }
            }
            content = result;
        }

        if (m_impl->m_mode == EquationMode::Display) {
            return "<m:oMathPara><m:oMath>" + content + "</m:oMath></m:oMathPara>";
        }
        return "<m:oMath>" + content + "</m:oMath>";
    }

} // namespace xword