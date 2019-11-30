
// see http://blog.madhukaraphatak.com/functional-programming-in-c++/

// 模板类 Best Practice，声明和实现都写在 .h 里。
// See: http://www.cs.technion.ac.il/users/yechiel/c++-faq/templates-defn-vs-decl.html

#ifndef COMBINATOR
#define COMBINATOR

#include <algorithm>
#include <functional>
#include <numeric>
#include <vector>

template <class Collection>
class Combinator {
 public:
  Combinator(const Collection& col) {
    this->real = col;  // copy collection
  };

  template <class lamda>
  void for_each(const lamda& op) {
    std::for_each(real.begin(), real.end(), op);
  }

  template <class lamda>
  Combinator<Collection>& map(const lamda& op) {
    std::transform(real.begin(), real.end(), real.begin(), op);
    return *this;
  }

  template <class Predicate>
  Combinator<Collection>& filter(const Predicate& predicate) {
    // capture the predicate in order to be used inside function
    auto fnCol = this->filterNot([predicate](typename Collection::value_type i) { return !predicate(i); }).collect();
    return *this;
  }

  // 以下 ReturnType 实现参考了: https://stackoverflow.com/questions/21462974/return-type-of-a-c-lambda
  // 希望会有更好的实现
  template <class first_getter_lamda, class ReturnType, class lamda>
  ReturnType reduce_first(const first_getter_lamda& firstLamda, const ReturnType& type, const lamda& op) {
    return std::accumulate(std::next(real.begin()), real.end(), firstLamda(real[0]), op);
  }

  ///////// 归约操作 /////////

  template <class Predicate>
  bool all(const Predicate& predicate) {
    return (std::all_of(real.cbegin(), real.cend(), predicate));
  }

  template <class Predicate>
  bool none(const Predicate& predicate) {
    return (std::none_of(real.cbegin(), real.cend(), predicate));
  }

  template <class Predicate>
  bool any(const Predicate& predicate) {
    return (std::any_of(real.cbegin(), real.cend(), predicate));
  }

  template <class Accumulator, class lamda>
  Accumulator reduce(const Accumulator& accumulator, const lamda& op) {
    return std::accumulate(real.begin(), real.end(), accumulator, op);
  }

  Collection& collect() {
    return real;  // 返回 copy 值，防止外面破坏数据？
  }

  template <class ReturnType>
  ReturnType join(std::function<ReturnType(const typename Collection::value_type& ele)> joiner) {
    // TODO
    // E join(const std::vector<E>& v, const std::string& separator) {
    //   using namespace std;
    //   return accumulate(next(v.begin()), v.end(), v[0],
    //                     [&separator](const E& accu, const E& element) -> E { return accu + separator + element; });
    // }
  }

  ~Combinator() {
    // out("Combinator released");
  }

 private:
  Collection real;

  template <class Predicate>
  Combinator<Collection>& filterNot(const Predicate& predicate) {
    auto returnIterator = std::remove_if(real.begin(), real.end(), predicate);
    real.erase(returnIterator, std::end(real));
    return *this;
  }
};

#endif
