
#include <sstream>
#include <string>

#include <iostream>
#include <memory>

#ifndef UTILS_STR
#define UTILS_STR

// std::string 没有类似的方法，see:
// https://stackoverflow.com/questions/7755719/check-if-string-starts-with-another-string-find-or-compare

bool str_starts_with(const std::string &str, const std::string &prefix);

bool str_ends_with(const std::string &str, const std::string &suffix);

// 不会修改原始字符串
std::string string_to_upper(std::string strToConvert);

// 不会修改原始字符串
std::string string_to_lower(std::string strToConvert);

// usage:
//   std::string foo = string_format("aaa %d", 123);
template <typename... Args>
std::string string_format(const std::string &format, Args... args) {
  std::size_t size = snprintf(nullptr, 0, format.c_str(), args...) + 1;  // Extra space for '\0'
  std::unique_ptr<char[]> buf(new char[size]);
  snprintf(buf.get(), size, format.c_str(), args...);
  return std::string(buf.get(), buf.get() + size - 1);  // We don't want the '\0' inside
};

// mutable! 修改原始 str
//
// usage:
//   replaceAll(content, "$name", "Somename");
bool replaceAll(std::string &str, const std::string &from, const std::string &to);

std::string url_encode(const std::string &s);

#endif
