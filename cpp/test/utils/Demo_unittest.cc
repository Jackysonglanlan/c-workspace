
#include "Demo.cpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace std;

namespace {


///////////////////////////
////// Test Fixture ///////
///////////////////////////

class DemoTest : public ::testing::TestWithParam<std::string> {
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

  DemoTest() {
    using namespace std;
    const ParamType testParam = GetParam();
    //
  }

  ~DemoTest() {
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

using ParamType         = std::string;  // 请按测试需要修改 Value-Parameterized 类型
ParamType valueParams[] = {"foo"};      // 请按测试需要修改 测试参数

INSTANTIATE_TEST_CASE_P(Demo_UnitTest, DemoTest, ::testing::ValuesIn(valueParams));


///////////////////////////
/////// Assertions ////////
///////////////////////////

/**
 * 请按测试需要修改，输出更有意义的报错信息
 *
 * 用法:
 *   EXPECT_TRUE(isEven(3)); // 输出写入 AssertionFailure() 的数据
 *   EXPECT_FALSE(isEven(4)); // 输出写入 AssertionSuccess() 的数据
 */
const auto isEven = [](int n) -> ::testing::AssertionResult {
  if ((n % 2) == 0) {
    return ::testing::AssertionSuccess() << n << " is even";
  } else {
    return ::testing::AssertionFailure() << n << " is odd";
  }
};


/////////////////////////
/////// Matchers ////////
/////////////////////////

// Equal
using ::testing::AllOf;
using ::testing::AnyNumber;
using ::testing::Eq;
using ::testing::Ge;
using ::testing::Gt;
using ::testing::IsNull;
using ::testing::Le;
using ::testing::Lt;
using ::testing::Ne;
using ::testing::NotNull;
using ::testing::Optional;
using ::testing::Ref;
using ::testing::TypedEq;
using ::testing::VariantWith;

// float
using ::testing::DoubleEq;
using ::testing::DoubleNear;
using ::testing::FloatEq;
using ::testing::FloatNear;
using ::testing::NanSensitiveDoubleEq;
using ::testing::NanSensitiveDoubleNear;
using ::testing::NanSensitiveFloatEq;
using ::testing::NanSensitiveFloatNear;

// string
using ::testing::ContainsRegex;
using ::testing::EndsWith;
using ::testing::HasSubstr;
using ::testing::MatchesRegex;
using ::testing::StartsWith;
using ::testing::StrCaseEq;
using ::testing::StrCaseNe;
using ::testing::StrEq;
using ::testing::StrNe;

// container
using ::testing::ContainerEq;
using ::testing::Contains;
using ::testing::Each;
using ::testing::ElementsAre;
using ::testing::ElementsAreArray;
using ::testing::IsEmpty;
using ::testing::Pointwise;
using ::testing::SizeIs;
using ::testing::UnorderedElementsAre;
using ::testing::WhenSorted;
using ::testing::WhenSortedBy;

/**
 * Matcher 用法: 可以用在 ASSERT_THAT / EXPECT_THAT / EXPECT_CALL 中
 *
 * E.g:
 *
 *   EXPECT_THAT("hello_world", StartsWith("hello"));
 *
 *   // 结合 mock
 *   EXPECT_CALL(foo, methodXXX( AllOf(Gt(5),Ne(10)) )); // foo.methodXXX() 接收的参数 must be > 5 and != 10.
 *   EXPECT_CALL(foo, InRange( Ne(0), _ )).With(Lt());
 *   EXPECT_CALL(foo, methodXXX(Lt(5))).WillRepeatedly(Return('a'));
 *
 * 速查文档:
 *
 * Matcher API:
 *   https://github.com/abseil/googletest/blob/master/googlemock/docs/CheatSheet.md
 *
 * Matcher UserGuide:
 *   https://github.com/abseil/googletest/blob/master/googlemock/docs/CookBook.md#using-matchers
 */


/////////////////////////
/////// Mocking  ////////
/////////////////////////

using ::testing::AnyNumber;
using ::testing::AtLeast;
using ::testing::AtMost;
using ::testing::Between;
using ::testing::Exactly;

using ::testing::_;

using ::testing::Invoke;
using ::testing::MockFunction;

/**
 * Mock，行为代理到真正的对象，由此你可以测试 调用时参数/顺序/次数/...，而这个 Mock 的输出却是和生产环境一样！
 *
 * 用法:
 *
 *   DemoMock mock;
 *   EXPECT_CALL(mock, methodXXX()).Times(3);
 *   // 使用 mock
 *   // ...
 */

// 请按测试需要修改
class DemoMock : public Demo {
 public:
  DemoMock() {
    // 需要代理到真正的对象来执行的方法

    ON_CALL(*this, anotherNoParam()).WillByDefault(Invoke(&_real, &Demo::anotherNoParam));
    ON_CALL(*this, bar(_, _)).WillByDefault(Invoke(&_real, &Demo::bar));
    ON_CALL(*this, haha(_)).WillByDefault(Invoke(&_real, &Demo::haha));
    ON_CALL(*this, noParam()).WillByDefault(Invoke(&_real, &Demo::noParam));
  }

  // 需要拿 Mock 封装一层的方法，这些方法会被 Mock 检验调用状态

  MOCK_METHOD0(anotherNoParam, void());
  MOCK_METHOD2(bar, float(int a, float b));
  MOCK_METHOD1(haha, void(std::string msg));
  MOCK_METHOD0(noParam, void(void));

 private:
  // 初始化真正的对象
  Demo _real;
};


/////////////////////////
////// Unit Test  ///////
/////////////////////////

// test for: void anotherNoParam()
TEST_P(DemoTest, anotherNoParam) {
  using namespace std;

  const ParamType& testParam = GetParam();

  // 默认模式 Mock，没有被 EXPECT_CALL 的方法会打印 warning，请按测试需要修改
  DemoMock mock;
  // 配置/使用 mock
}

// test for: float bar(int a, float b)
TEST_P(DemoTest, bar) {
  using namespace std;

  const ParamType& testParam = GetParam();

  // 默认模式 Mock，没有被 EXPECT_CALL 的方法会打印 warning，请按测试需要修改
  DemoMock mock;
  // 配置/使用 mock
}

// test for: void haha(std::string msg)
TEST_P(DemoTest, haha) {
  using namespace std;

  const ParamType& testParam = GetParam();

  // 默认模式 Mock，没有被 EXPECT_CALL 的方法会打印 warning，请按测试需要修改
  DemoMock mock;
  // 配置/使用 mock
}

// test for: void noParam(void)
TEST_P(DemoTest, noParam) {
  using namespace std;

  const ParamType& testParam = GetParam();

  // 默认模式 Mock，没有被 EXPECT_CALL 的方法会打印 warning，请按测试需要修改
  DemoMock mock;
  // 配置/使用 mock
}


}  // namespace
