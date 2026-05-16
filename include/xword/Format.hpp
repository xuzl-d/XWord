#pragma once

#include <cstdarg>
#include <cstdio>
#include <string>
#include <utility>

namespace xword {

// printf-style format helper. Useful for building LaTeX equation strings
// where {} is reserved by LaTeX (so std::format's {{}} escaping is noisy):
//
//   doc.addDisplayEquation(
//       xword::format("\\frac{a}{b} = %.3f", a / b));
//
// '%' is rare in LaTeX expressions (it's a comment marker outside math), so
// in practice no escaping is needed. To emit a literal '%', write "%%".
template<typename... Args>
inline std::string format(const char* fmt, Args&&... args) {
    int n = std::snprintf(nullptr, 0, fmt, std::forward<Args>(args)...);
    if (n <= 0) return {};
    std::string s(static_cast<size_t>(n), '\0');
    std::snprintf(s.data(), static_cast<size_t>(n) + 1, fmt, std::forward<Args>(args)...);
    return s;
}

} // namespace xword
