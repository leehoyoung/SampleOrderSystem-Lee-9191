#pragma once

#include <algorithm>
#include <cctype>
#include <string>

namespace StringUtil {

// 파이프/리다이렉션 입력 등에서 섞여 들어올 수 있는 개행("\r")과 앞뒤 공백을 제거한다.
inline std::string trim(const std::string& s) {
    const auto isSpace = [](unsigned char c) { return std::isspace(c) != 0; };

    auto begin = std::find_if_not(s.begin(), s.end(), isSpace);
    auto end = std::find_if_not(s.rbegin(), s.rend(), isSpace).base();

    return (begin < end) ? std::string(begin, end) : std::string();
}

}  // namespace StringUtil
