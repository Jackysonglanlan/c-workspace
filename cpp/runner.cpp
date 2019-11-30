

#include <iostream>

#include "src/utils/combinator/Combinator.h"
#include "src/utils/utils.h"

#include <sstream>
#include "date/date.h"

using namespace std;

void try_date();

void p(const stringstream& ss) {
  out("--------");
  out(ss.str());
  out("--------");
}

void run() {
  vector<int> col{20, 24, 37, 42, 23, 45, 17};

  Combinator<vector<int>> comb(col);

  // comb.for_each([](const int e) -> void { cout << e << " | "; });

  cout << comb.map([](const int e) -> int { return e + 1; })
              .filter([](const int e) -> bool { return e % 2 == 0; })
              .collect();

  try_date();
}

void try_date() {
  using namespace date;
  using namespace std::chrono;
  auto today = floor<days>(system_clock::now());

  stringstream* ss = new stringstream();
  *ss << today;
  p(*ss);
  delete ss;
}
