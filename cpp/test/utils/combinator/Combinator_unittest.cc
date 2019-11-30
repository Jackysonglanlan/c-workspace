
#include "src/utils/combinator/Combinator.h"

#include "gtest/gtest.h"


class CombinatorTest : public ::testing::Test {
 protected:
  CombinatorTest() {
    col  = new std::vector<int>{20, 24, 37, 42, 23, 45, 17};
    comb = new Combinator<std::vector<int>>(*col);
  }

  ~CombinatorTest() {
    delete col;
    delete comb;
  }

  void SetUp() override {
    // Code here will be called immediately after the constructor (right before each test).
  }

  void TearDown() override {
    // Code here will be called immediately after each test (right before the destructor).
  }

  // Objects declared here can be used by all tests in the test case for Foo.
  std::vector<int> *col;
  Combinator<std::vector<int>> *comb;
};

namespace {

TEST_F(CombinatorTest, All) {
  using namespace std;

  EXPECT_FALSE(comb->all([](const int e) -> bool { return e > 30; }));
}

TEST_F(CombinatorTest, ForEach) {
  using namespace std;

  int count = 0;
  comb->for_each([&count, this](const int e) -> void { EXPECT_EQ(e, (*col)[count++]); });
}

TEST_F(CombinatorTest, Map) {
  using namespace std;

  EXPECT_EQ(*col, comb->map([](const int e) -> int { return e + 0; }).collect());
}

TEST_F(CombinatorTest, ReduceFirst) {
  using namespace std;

  EXPECT_EQ("20 + 24 + 37 + 42 + 23 + 45 + 17",
            comb->reduce_first([](const int first) -> string { return to_string(first); }, string(),
                               [](string accu, const int e) -> string { return accu + " + " + to_string(e); }));
}

TEST_F(CombinatorTest, Reduce) {
  using namespace std;

  EXPECT_EQ(209, comb->reduce(1, [](int accu, const int e) -> int { return accu + e; }));
}

}  // namespace
