#pragma once

#include <cstdio>
#include <string>
#include <utility>

namespace xword {

/// printf-style format helper for building LaTeX equation strings.
///
/// Unlike `std::format`, this uses `%` placeholders so that LaTeX `{` `}`
/// braces do not need escaping:
///
///     doc.addDisplayEquation(
///         xword::format("\\frac{a}{b} = %.3f", a / b));
///
/// `%` is rare in LaTeX math; to emit a literal `%` write `%%`.
template<typename... Args>
inline std::string format(const char* fmt, Args&&... args) {
    int n = std::snprintf(nullptr, 0, fmt, std::forward<Args>(args)...);
    if (n <= 0) return {};
    std::string s(static_cast<size_t>(n), '\0');
    std::snprintf(s.data(), static_cast<size_t>(n) + 1, fmt,
                  std::forward<Args>(args)...);
    return s;
}

} // namespace xword
