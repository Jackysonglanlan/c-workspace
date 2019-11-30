
#include "Debug.h"

#include <chrono>
// #include "../date/chrono_io.h"

using namespace std;

void debug_t(const std::string& msg, const std::function<void()>& lamda) {
  // using Clock = std::chrono::steady_clock;  // 绝大部分时间使用这个(OS 不会调整这个时间，见文档)

  // auto t1 = Clock::now();
  // lamda();
  // auto t2 = Clock::now();

  // std::cout << "⏳ " << msg << " : [" << t2 - t1 << "] ⏳" << std::endl;
}
