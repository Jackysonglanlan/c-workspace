
#include "src/utils/debug/Debug.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace std;

namespace {

///////////////////////////
////// Test Fixture ///////
///////////////////////////

using ParamType         = std::string;  // 请按测试需要修改 Value-Parameterized 类型
ParamType valueParams[] = {"foo"};      // 请按测试需要修改 测试参数

class DebugTest : public ::testing::TestWithParam<std::string> {
 public:
  // Per-test-case set-up. Called before the first test in this test case.
  static void SetUpTestCase() {
    using namespace std;
    // shared_resource_ = new...;
  }

  // Per-test-case tear-down. Called after the last test in this test case.
  static void TearDownTestCase() {
    using namespace std;
    // delete shared_resource_;
    // shared_resource_ = NULL;
  }

 protected:
  // Some expensive resource shared by all tests.
  // static T* shared_resource_;

  DebugTest() {
    using namespace std;
    const ParamType testParam = GetParam();
    //
  }

  ~DebugTest() {
    //
  }

  void SetUp() override {
    using namespace std;
    const ParamType testParam = GetParam();
    // Code here will be called immediately after the constructor (right before each test).
  }

  void TearDown() override {
    using namespace std;
    const ParamType testParam = GetParam();
    // Code here will be called immediately after each test (right before the destructor).
  }

  // Objects declared here can be used by all tests in the test case for Foo.
};

// T* PipeTest::shared_resource_ = NULL;

INSTANTIATE_TEST_CASE_P(Debug_UnitTest, DebugTest, ::testing::ValuesIn(valueParams));

/////////////////////////
////// Unit Test  ///////
/////////////////////////

string rhs() { return "rhs"; }

// test for: debug
TEST_P(DebugTest, debug) {
  using namespace std;
  const ParamType testParam = GetParam();

  debug(testParam);
  debug(rhs());

  debug(true, testParam);
  debug(true, rhs());
  debug(false, testParam);
  debug(false, rhs());
}

TEST_P(DebugTest, debug_t) {
  using namespace std;
  bool isLamdaRun = false;
  debug_t("Duration of some code", [&isLamdaRun]() -> void { isLamdaRun = true; });
  EXPECT_TRUE(isLamdaRun);
}

}  // namespace
