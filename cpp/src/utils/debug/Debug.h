

#ifndef JSDEBUG
#define JSDEBUG

#include <functional>
#include <iostream>
#include <string>

/**
 * 控制台输出
 */
template <class E>
void debug(const E &content) {  // const refer，可以同时支持 左值 和 右值
  using namespace std;
  std::cout << "🔥";  // 使用 emoji 方便在控制台数据太多时，迅速看到打印的值
  std::cout << (content);
  std::cout << "🔥" << std::endl;
}

/**
 * assertTrue == true 时才输出，方便 当数据很多的时候 精确打印
 */
template <class E>
void debug(bool assertTrue, const E &content) {
  if (assertTrue) {
    debug(content);
  }
}

/**
 * 控制台输出，计算 lamda 的运行时间，精确到 毫秒
 *
 */
void debug_t(const std::string &msg, const std::function<void()>& lamda);

#endif
