
/*
 * 自动生成 gtest 的 stub 测试代码
 *
 * run:
 *   g++ --std=c++11 gene-gtest-stub.cpp -o gene && ./gene "path/to/ctags-symbol-file" "path-to-your-class" && rm gene
 *
 * 比如有一个类 utils/pipe/Pipe.h, 会生成 test/utils/pipe/Pipe_unittest.cc 文件，内容如下:
 *
 * include "src/utils/pipe/Pipe.h"
 * include "gtest/gtest.h"
 * include "gmock/gmock.h"
 *
 * namespace {
 *   TEST_P(_ClassName_Test, _methodName_){
 *     using namespace std;
 *
 *   }
 *   TEST_P(..., _method2Name_){ }
 *   TEST_P(..., _method3Name_){ }
 *   ...
 *   TEST_P(..., _method4Name_){ }
 * }
 *
 * 实现原理:
 * 读取 ctags 文件，解析后拼出来，ctags 内容类似如下:
 *
 * 比如对于 utils/pipe/Pipe.h 文件有
 *
 * PIPE  utils/pipe/Pipe.h /^#define PIPE$/;"  d
 * Pipe  utils/pipe/Pipe.h /^  Pipe() {$/;"  f class:Pipe
 * Pipe  utils/pipe/Pipe.h /^class Pipe {$/;"  c
 * _steps  utils/pipe/Pipe.h /^  std::map<uint, STEP<T, ReturnType>> *_steps;  [step order num -> STEP]$/;" m class:Pipe
 * ~Pipe utils/pipe/Pipe.h /^  ~Pipe() { delete _steps; }$/;"  f class:Pipe
 *
 * 1. 利用 ctags 拿到这个文件相关的符号
 * 2. 有 f class: 字样的行，代表是一个方法，可以根据这个进行过滤，拿到所有方法
 * 3. 解析出方法名，生成类似如下代码
 *
 * TEST_P(_FileName_Test, _methodName_){
 *   using namespace std;
 *
 * }
 *
 * 4. 生成最终的测试文件 stub，由于输入参数里有文件名，所以连 测试文件名和所在路径 都可以一起生成
 */

#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <regex>
#include <string>
#include <vector>

using namespace std;

/*
void c_read_ctags(const char* ) {
  FILE* fp = fopen(FILENAME, "r");
  if (fp == NULL) exit(EXIT_FAILURE);

  char* line = NULL;
  size_t len = 0;
  while ((getline(&line, &len, fp)) != -1) {
    // using printf() in all tests for consistency
    printf("%s", line);
  }
  fclose(fp);
  if (line) free(line);
}
*/

//////// helpers /////////


template <class E>
void debug(const E& content);

template <class E>
E join(const std::vector<E>& v, const std::string& separator) {
  using namespace std;
  return accumulate(next(v.begin()), v.end(), v[0],
                    [&separator](const E& accu, const E& element) -> E { return accu + separator + element; });
}

template <class E>
std::ostream& operator<<(std::ostream& os, const std::vector<E>& v) {
  using namespace std;
  std::cout << ("\n-------------\n");
  for_each(v.begin(), v.end(), [](const E& e) { cout << e << endl; });
  std::cout << ("\n-------------\n");
  return os;
}

template <class K, class V>
std::ostream& operator<<(std::ostream& os, const std::map<K, V>& map) {
  using namespace std;
  std::cout << ("{");
  for (auto& iter : map) {
    cout << "  " << iter.first << " -> " << iter.second << endl;
  }
  std::cout << ("}");
  return os;
}

template <class E>
void debug(const E& content) {
  std::cout << "🔥";
  std::cout << (content);
  std::cout << "🔥" << std::endl;
}

// replaceAll(content, "$name", "Somename");
bool replaceAll(std::string& str, const std::string& from, const std::string& to) {
  size_t start_pos = str.find(from);
  if (start_pos == std::string::npos) return false;

  str.replace(start_pos, from.length(), to);
  return replaceAll(str, from, to);  // 尾递归
}

template <typename... Args>
string string_format(const std::string& format, Args... args) {
  size_t size = snprintf(nullptr, 0, format.c_str(), args...) + 1;  // Extra space for '\0'
  unique_ptr<char[]> buf(new char[size]);
  snprintf(buf.get(), size, format.c_str(), args...);
  return string(buf.get(), buf.get() + size - 1);  // We don't want the '\0' inside
}


//////// private /////////

using CTagsLines = vector<string>;

using MethodSymbols = map<string, string>;

string reduceMethodSymbols(
    const MethodSymbols& symbols,
    std::function<string(const string& accu, const string& methodName, const string& methodSignature)> lamda) {
  return std::accumulate(symbols.begin(), symbols.end(), string(),
                         [&lamda](const string& accu, const std::pair<string, string>& symbol) -> string {
                           return lamda(accu, symbol.first, symbol.second);
                         });
}

string gene_gtest_file_path(const string& origSrcFilePath) {
  std::smatch match;
  std::regex_search(origSrcFilePath, match, std::regex("(.+)\\..+"));
  // \1: file path without extension
  return "test/" + match.str(1) + "_unittest.cc";
}

CTagsLines find_symbol_lines_in_ctags(const string& ctagsFileName, const string& targetFilePath) {
  std::ifstream file(ctagsFileName);

  CTagsLines neededCTagsLines;
  if (!file.is_open()) {
    return neededCTagsLines;
  }

  for (std::string line; getline(file, line);) {
    if (line.find(targetFilePath) != std::string::npos) {
      neededCTagsLines.emplace_back(line);
    }
  }
  file.close();
  return neededCTagsLines;
}

string gene_gtest_test_fixture_class_stub(const string& origClassName, const string& testClassName) {
  string stub = R"EOF(
    ///////////////////////////
    ////// Test Fixture ///////
    ///////////////////////////

    class ${testClassName} : public ::testing::TestWithParam<std::string> {
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

      ${testClassName}() {
        using namespace std;
        const ParamType testParam = GetParam();
        //
      }

      ~${testClassName}() {
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
    ParamType valueParams[] = {"foo"};    // 请按测试需要修改 测试参数

    INSTANTIATE_TEST_CASE_P(${origClassName}_UnitTest, ${testClassName}, ::testing::ValuesIn(valueParams));
  )EOF";

  replaceAll(stub, "${testClassName}", testClassName);
  replaceAll(stub, "${origClassName}", origClassName);

  return stub;
}

string gene_assertions_stub() {
  return R"EOF(
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
  )EOF";
}

string gene_matcher_stub() {
  return R"EOF(
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
  )EOF";
}

/**
 * 计算方法参数个数
 *
 * @param  methodSignature Like: void haha(std::string msg)
 *
 * @return                   参数个数
 */
uint countMethodParams(const string& methodSignature) {
  uint paramNum = std::count(methodSignature.begin(), methodSignature.end(), ',');

  //// 分析方法参数数量，看 "," 数量

  // # 有 ","，参数数量为 个数 + 1
  if (paramNum > 0) {
    return paramNum + 1;
  }

  // # 如果没有 ","
  // # 判断方法参数签名是否是 () 或 (void)
  std::smatch match;
  if (std::regex_match(methodSignature, match, std::regex(".+\\( *(void)? *\\).*"))) {
    return 0;  // # 如果是，就没有参数
  }

  return 1;  // # 否则就是 1 个参数
}

/**
 * 生成 ON_CALL 的默认参数列表，格式: _,_,_,...
 *
 * see https://github.com/abseil/googletest/blob/master/googlemock/docs/CookBook.md#delegating-calls-to-a-real-object
 *
 * @param  paramNum 参数个数
 *
 * @return          _,_,_,...
 */
string gene_onCall_method_param_sig(uint paramNum) {
  if (paramNum == 0) {
    return "";
  }

  string signature = string_format("%0" + std::to_string(paramNum) + "c", '0');
  replaceAll(signature, "0", "_,");
  // C++11 专用，删除最后一个字符(",")
  // see https://stackoverflow.com/questions/2310939/remove-last-character-from-c-string
  signature.pop_back();

  // debug(" --> " + signature);
  return signature;
}

string gene_mock_stub(const string& origClassName, const string& mockClassName, const MethodSymbols& symbols) {
  // 生成 ON_CALL 语句，格式见:
  //   https://github.com/abseil/googletest/blob/master/googlemock/docs/CookBook.md#delegating-calls-to-a-real-object
  const auto onCallStub = reduceMethodSymbols(
      symbols, [&origClassName](const string& accu, const string& methodName, const string& methodSignature) -> string {
        uint methodParamCount       = countMethodParams(methodSignature);
        string onCallMethodParamSig = gene_onCall_method_param_sig(methodParamCount);

        string onCallTemplate = R"EOF(
                              ON_CALL(*this, __method_name__(__on_call_method_param_sig__))
                              .WillByDefault(Invoke(&_real, &$origClassName::__method_name__));)EOF";

        replaceAll(onCallTemplate, "__method_name__", methodName);
        replaceAll(onCallTemplate, "__on_call_method_param_sig__", onCallMethodParamSig);
        replaceAll(onCallTemplate, "$origClassName", origClassName);

        return accu + onCallTemplate;
      });

  const auto mockMethodStub = reduceMethodSymbols(
      symbols, [](const string& accu, const string& methodName, const string& methodSignature) -> string {
        uint methodParamCount = countMethodParams(methodSignature);

        // 生成 MOCK_METHOD 语句，MOCK_METHOD 后面的数字是要 mock 的方法参数个数
        string mockMethodTemplate = R"EOF(
          MOCK_METHOD__method_param_count__(__method_name__, __method_signature__);)EOF";

        // 这里的方法签名要去掉方法名，见 文档
        // https://github.com/abseil/googletest/blob/master/googlemock/docs/CookBook.md#delegating-calls-to-a-real-object
        auto sigCopy = methodSignature;
        replaceAll(sigCopy, methodName, "");

        replaceAll(mockMethodTemplate, "__method_name__", methodName);
        replaceAll(mockMethodTemplate, "__method_signature__", sigCopy);
        replaceAll(mockMethodTemplate, "__method_param_count__", std::to_string(methodParamCount));

        return accu + mockMethodTemplate;
      });


  string mockStub = R"EOF(
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
     *   ${mockClassName} mock;
     *   EXPECT_CALL(mock, methodXXX()).Times(3);
     *   // 使用 mock
     *   // ...
     */

    // 请按测试需要修改
    class ${mockClassName} : public ${origClassName} {
     public:
      ${mockClassName}() {
        // 需要代理到真正的对象来执行的方法
        ${onCallStub}
      }

      // 需要拿 Mock 封装一层的方法，这些方法会被 Mock 检验调用状态
      ${mockMethodStub}

     private:
      // 初始化真正的对象
      ${origClassName} _real;
    };
  )EOF";

  replaceAll(mockStub, "${mockClassName}", mockClassName);
  replaceAll(mockStub, "${origClassName}", origClassName);
  replaceAll(mockStub, "${onCallStub}", onCallStub);
  replaceAll(mockStub, "${mockMethodStub}", mockMethodStub);

  // debug(mockStub);

  return mockStub;
}

string gene_gtest_method_stub(const string& testClassName, const string& mockClassName,
                              const MethodSymbols& methodSymbols) {
  const auto methodStub =
      reduceMethodSymbols(methodSymbols,
                          [&testClassName, &mockClassName](const string& accu, const string& methodName,
                                                           const string& methodSignature) -> string {
                            string stubCopy = R"EOF(
                              // test for: __method_signature__
                              TEST_P(${testClassName}, __method_name__) {
                                using namespace std;

                                const ParamType& testParam = GetParam();

                                // 默认模式 Mock，没有被 EXPECT_CALL 的方法会打印 warning，请按测试需要修改
                                ${mockClassName} mock;
                                // 配置/使用 mock
                              }
                            )EOF";
                            // # ParamType 在 _gene_test_fixture_class_stub() 中引入
                            // # $mockClassName 在 _gene_mock_stub() 中引入

                            replaceAll(stubCopy, "__method_name__", methodName);
                            replaceAll(stubCopy, "__method_signature__", methodSignature);
                            replaceAll(stubCopy, "${testClassName}", testClassName);
                            replaceAll(stubCopy, "${mockClassName}", mockClassName);

                            return accu + stubCopy;
                          });

  return R"EOF(
    /////////////////////////
    ////// Unit Test  ///////
    /////////////////////////
  )EOF" + methodStub;
}

/**
 * 把所有的 stub 块 拼在一起，生成完整的 GTest 测试文件源代码
 *
 * @param  origSrcFilePath 要测试的源文件路径
 * @param  subBlockList    "Assertion" "Mock" 等所有的 stub 块
 *
 * @return                 完整的 GTest 测试文件源代码
 */
string gene_gtest_file_stub(const std::string& origSrcFilePath, std::initializer_list<string> subBlockList) {
  std::vector<string> list = subBlockList;

  string stub = R"EOF(
    #include "${origSrcFilePath}"
    #include "gmock/gmock.h"
    #include "gtest/gtest.h"

    using namespace std;

    namespace {

      ${stubBlockList}

    } // namespace
  )EOF";

  replaceAll(stub, "${origSrcFilePath}", origSrcFilePath);
  replaceAll(stub, "${stubBlockList}", join(list, "\n"));

  return stub;
}

void output_gtest_stub_file_to_path(const string& gtestFilePath, const string& gtestFileStub) {
  //
  // debug(gtestFileStub);
  // debug(gtestFilePath);

  ofstream gtestFile(gtestFilePath);
  if (!gtestFile.is_open()) {
    cout << ("[Error] Abort! Can't open file " + gtestFilePath) << endl;
    exit(1);
  }

  // 这里注意，即使不往文件里写，只要打开成功，文件内容已经被清空了!

  gtestFile << gtestFileStub;
  gtestFile.close();
}

string get_class_name_from_path(const string& origSrcFilePath) {
  //
  std::smatch match;
  std::regex_search(origSrcFilePath, match, std::regex(".+/(.+?)\\..+"));
  return match.str(1);
}

/**
 * Parse out *all* the method's symbols, exclude dtoc.
 *
 * @parameters lines Ctags file lines, like this:
 *                   start  utils/pipe/Pipe.h /^  ReturnType start(const T &initValue) {$/;"  f class:Pipe
 *
 * return            {method name -> method signature}
 */
MethodSymbols parse_all_method_symbols_from_ctags(CTagsLines& lines) {
  const std::regex funcLineRegex("\tf\tclass:.+");
  const std::regex dtocRegex("~.+?\\(\\)");

  // only need method lines
  auto removeIter =
      std::remove_if(lines.begin(), lines.end(), [&funcLineRegex, &dtocRegex](const string& line) -> bool {
        std::smatch match;
        bool isNotFunctionLine = !(std::regex_search(line, match, funcLineRegex) && match.size() == 1);
        bool isDtorLine        = (std::regex_search(line, match, dtocRegex) && match.size() == 1);
        return isNotFunctionLine || (isDtorLine);
      });
  lines.erase(removeIter, std::end(lines));

  const auto& signatureRegex = std::regex("^(.+?)\t.+?/\\^  (.+?)\\{.+");  // 1: name, 2: signature
  return std::accumulate(std::next(lines.begin()), lines.end(), MethodSymbols(),
                         [&signatureRegex](MethodSymbols& accu, const string& line) -> MethodSymbols {
                           std::smatch match;
                           std::regex_search(line, match, signatureRegex);
                           accu.emplace(match.str(1), match.str(2));
                           return accu;
                         });
}

void makesure_file_not_exist(const string& gtestFilePath) {
  ifstream gtestFile(gtestFilePath);
  if (gtestFile.is_open()) {
    cout << ("[Warning] Abort! Test file " + gtestFilePath + " already exist.") << endl;
    exit(1);
  }
}

//////// main /////////

int main(int argc, char const* argv[]) {
  // 第一个参数是 执行文件 本身的名字
  if (argc != 3) {
    // Tell the user how to run the program
    std::cerr << "Usage: ./gene-gtest-file ctags-symbol-file path-to-file" << std::endl;
    return 1;
  }
  const string ctagsSymbolFile = argv[1];
  const string origSrcFilePath = argv[2];

  const string gtestFilePath = gene_gtest_file_path(origSrcFilePath);

  makesure_file_not_exist(gtestFilePath);

  auto lines = find_symbol_lines_in_ctags(ctagsSymbolFile, origSrcFilePath);

  if (lines.size() == 0) {
    std::cerr << "[Error] Can't generate GTest file for: " << origSrcFilePath << std::endl;
    std::cerr << "        The file's symbols is NOT found in " << ctagsSymbolFile << std::endl;
    return 1;
  }

  auto className     = get_class_name_from_path(origSrcFilePath);
  auto mockClassName = className + "Mock";
  auto testClassName = className + "Test";

  auto methodSymbols = parse_all_method_symbols_from_ctags(lines);

  auto testFixtureClassStub = gene_gtest_test_fixture_class_stub(className, testClassName);
  auto assertStub           = gene_assertions_stub();
  auto matcherStub          = gene_matcher_stub();
  auto mockStubCode         = gene_mock_stub(className, mockClassName, methodSymbols);
  auto testMethodsStub      = gene_gtest_method_stub(testClassName, mockClassName, methodSymbols);

  auto gtestFileStub = gene_gtest_file_stub(origSrcFilePath,
                                            {
                                              testFixtureClassStub, assertStub, matcherStub,
                                              mockStubCode, testMethodsStub
                                            });

  output_gtest_stub_file_to_path(gtestFilePath, gtestFileStub);

  return 0;
}
