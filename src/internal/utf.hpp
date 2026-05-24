#pragma once

#include <string>
#include <locale>
#include <codecvt>

namespace xword {
namespace internal {

inline std::string wstring_to_utf8(const std::wstring& ws) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    return conv.to_bytes(ws);
}

} // namespace internal
} // namespace xword
