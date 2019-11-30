
#include "src/utils/pipe/Pipe.h"

#include "gtest/gtest.h"

namespace {

//////////////////////////
////// Test Config ///////
//////////////////////////

class PipeTest : public ::testing::TestWithParam<std::string> {
 public:
  // Per-test-case set-up.
  // Called before the first test in this test case.
  static void SetUpTestCase() {
    using namespace std;

    // shared_resource_ = new...;
  }

  // Per-test-case tear-down.
  // Called after the last test in this test case.
  static void TearDownTestCase() {
    using namespace std;

    // delete shared_resource_;
    // shared_resource_ = NULL;
  }
  // Some expensive resource shared by all tests.
  // static T* shared_resource_;

 protected:
  PipeTest() {
    using namespace std;
    //
  }

  ~PipeTest() {
    //
  }

  void SetUp() override {
    using namespace std;
    // Code here will be called immediately after the constructor (right before each test).
  }

  void TearDown() override {
    using namespace std;
    // Code here will be called immediately after each test (right before the destructor).
  }

  // Objects declared here can be used by all tests in the test case for Foo.
};

std::string valueParams[] = {"foo"};
INSTANTIATE_TEST_CASE_P(PipeUnitTest, PipeTest, ::testing::ValuesIn(valueParams));

// T* PipeTest::shared_resource_ = NULL;


////////////////////
////// Tests ///////
////////////////////

TEST_P(PipeTest, AddGetStep) {
  using namespace std;

  Pipe<string, string> pipe;

  pipe.addStepAt(1, [](const string& data) -> string { return data + " 111"; });

  EXPECT_EQ(pipe.getStepAt(1)("hahaha"), "hahaha 111");
}

TEST_P(PipeTest, Start) {
  using namespace std;

  Pipe<string, string> pipe;

  pipe.addStepAt(1, [](const string& data) -> string {
    //
    return data + " 111";
  });
  pipe.addStepAt(2, [](const string& data) -> string {
    //
    return data + " 222";
  });
  pipe.addStepAt(3, [](const string& data) -> string {
    //
    return data + " 333";
  });

  EXPECT_EQ(pipe.start("test string..."), "test string... 111 222 333");
}


}  // namespace
