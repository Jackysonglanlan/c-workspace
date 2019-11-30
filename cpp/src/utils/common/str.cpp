
#include "str.h"

#include <algorithm>
#include <functional>

using namespace std;

// std::string 没有类似的方法，see:
// https://stackoverflow.com/questions/7755719/check-if-string-starts-with-another-string-find-or-compare

bool str_starts_with(const std::string &str, const std::string &prefix) {
  return prefix.size() <= str.size() && str.compare(0, prefix.size(), prefix) == 0;
}

bool str_ends_with(const std::string &str, const std::string &suffix) {
  auto len = suffix.size();
  return suffix.size() <= str.size() && str.compare(str.size() - len, len, suffix) == 0;
}

std::string string_to_upper(std::string strToConvert) {  // copy 一次，因为 transform 是修改原始字符串，很危险
  std::transform(strToConvert.begin(), strToConvert.end(), strToConvert.begin(), ::toupper);

  return strToConvert;
}

std::string string_to_lower(std::string strToConvert) {  // copy 一次，因为 transform 是修改原始字符串，很危险
  std::transform(strToConvert.begin(), strToConvert.end(), strToConvert.begin(), ::tolower);

  return strToConvert;
}

bool replaceAll(std::string &str, const std::string &from, const std::string &to) {
  size_t start_pos = str.find(from);
  if (start_pos == std::string::npos) return false;

  str.replace(start_pos, from.length(), to);
  return replaceAll(str, from, to);  // 尾递归
}

std::string url_encode(const std::string &s) {
  static const char lookup[] = "0123456789abcdef";
  std::stringstream e;
  for (int i = 0, ix = s.length(); i < ix; i++) {
    const char &c = s[i];
    if ((48 <= c && c <= 57) ||   // 0-9
        (65 <= c && c <= 90) ||   // abc...xyz
        (97 <= c && c <= 122) ||  // ABC...XYZ
        (c == '-' || c == '_' || c == '.' || c == '~')) {
      e << c;
    } else {
      e << '%';
      e << lookup[(c & 0xF0) >> 4];
      e << lookup[(c & 0x0F)];
    }
  }
  return e.str();
}
