
#include <iostream>
#include <string>

#include <map>
#include <numeric>
#include <vector>

#ifndef UTILS_IO
#define UTILS_IO

#define out(content) (std::cout << (content) << std::endl)

/////////// 模板方法，声明和实现要写在一起

template <class E>
std::ostream &operator<<(std::ostream &os, const std::vector<E> &v) {
  using namespace std;
  cout << "\n-------------\n";
  for_each(v.begin(), v.end(), [](const E &e) { cout << e << " "; });
  cout << "\n-------------\n";
  return os;
}

template <class K, class V>
std::ostream &operator<<(std::ostream &os, const std::map<K, V> &map) {
  using namespace std;
  cout << "\n{\n";
  for (auto &iter : map) {
    cout << "  " << iter.first << " -> " << iter.second << endl;
  }
  cout << "\n}\n";
  return os;
}

#endif
