
#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <numeric>

#ifndef PIPE
#define PIPE

template <class T, class ReturnType>
using STEP = std::function<ReturnType(const T &param)>;

// 管道，可以作为 业务处理流水线 使用
// see https://medium.com/javascript-scene/composing-software-an-introduction-27b72500d6ea
// 等价的 js 实现:
// const pipe = (...fns) => x => fns.reduce((y, f) => f(y), x);
// usage: pipe(...fns: [...Function]) => x => y
template <class T, class ReturnType>
class Pipe {
 public:
  Pipe() {
    //
    _steps = new std::map<uint, STEP<T, ReturnType>>();
  }

  void addStepAt(uint order, const STEP<T, ReturnType> &step) { (*_steps).emplace(order, step); }

  void removeStepAt(uint order) { (*_steps).erase(order); }

  STEP<T, ReturnType> &getStepAt(uint order) {
    // 是否需要 copy?
    return (*_steps).at(order);
  }

  ReturnType start(const T &initValue) {
    auto &steps = (*_steps);
    return std::accumulate(steps.begin(), steps.end(), initValue, [](const T &accu, const auto &pair) -> ReturnType {
      // 第一个函数的输出作为第二个的输入，形成 pipe
      auto &step = pair.second;
      return step(accu);
    });
  }

  //
  ~Pipe() { delete _steps; }

 private:
  //
  std::map<uint, STEP<T, ReturnType>> *_steps;  // [step order num -> STEP]
};

#endif
