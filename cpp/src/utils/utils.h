
#include <iostream>
#include <sstream>

#include <string>

#include <algorithm>
#include <functional>
#include <map>
#include <numeric>
#include <vector>

//////// Bad Practice:
//
// using namespace std; // 在 .h 里面引用 using，会污染全局，std 又是最常用的一个，所以污染会更严重
//

#ifndef UTILS
#define UTILS

#include "utils/common/str.h"
#include "utils/debug/Debug.h"
#include "utils/io/io.h"
#include "utils/meta/meta.h"

#endif
