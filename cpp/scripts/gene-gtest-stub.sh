# !/bin/sh
#
# 自动生成 gtest 测试代码的 stub
#
# E.g:
#   ./run gene_gtest_stub utils/pipe/Pipe.h
#
# 会生成 test/utils/pipe/Pipe_unittest.cc 文件，内容如下:
#
# include "src/utils/pipe/Pipe.h"
# include "gtest/gtest.h"
# include "gmock/gmock.h"
#
# namespace {
#   TEST_P(_ClassName_Test, _methodName_){
#     using namespace std;
#
#   }
#   TEST_P(..., _method2Name_){ }
#   TEST_P(..., _method3Name_){ }
#   ...
#   TEST_P(..., _method4Name_){ }
# }
#
# 实现原理:
# 读取 ctags 文件，解析后拼出来，ctags 内容类似如下:
#
# 比如对于 utils/pipe/Pipe.h 文件有
#
# PIPE  utils/pipe/Pipe.h /^#define PIPE$/;"  d
# Pipe  utils/pipe/Pipe.h /^  Pipe() {$/;"  f class:Pipe
# Pipe  utils/pipe/Pipe.h /^class Pipe {$/;"  c
# _steps  utils/pipe/Pipe.h /^  std::map<uint, STEP<T, ReturnType>> *_steps;  [step order num -> STEP]$/;" m class:Pipe
# ~Pipe utils/pipe/Pipe.h /^  ~Pipe() { delete _steps; }$/;"  f class:Pipe
#
# 1. 利用 ctags 分析出这个文件相关的符号
# 2. 有 f class: 字样的行，代表是一个方法，可以根据这个进行过滤，拿到所有方法
# 3. 解析出方法名，生成类似如下代码
#
# TEST_P(_FileName_Test, _methodName_){
#   using namespace std;
#
# }
#
# 4. 生成最终的测试文件 stub，由于输入参数里有文件名，所以连 测试文件名和所在路径 都可以一起生成

set -euo pipefail
trap "echo 'error: Script failed: see failed command above'" ERR

DIR="${BASH_SOURCE%/*}"
if [[ ! -d "$DIR" ]]; then DIR="$PWD"; fi

source $DIR/utils.sh

use_red_green_echo "Generator"

###########
## private
###########

# 分隔符不能和 语言本身的方法命名字符 重复
METHOD_SEPARATOR="$"
METHOD_NAME_SIGNATURE_SEPARATOR="/"

# $1: origSrcFilePath
_parse_all_function_symbols_of_class(){
  local origSrcFilePath=$1
  
  # 利用 ctags 生成符号
  local metaData=$(ctags -f - "$origSrcFilePath")
  
  # 提取所有的方法: 所有包含 f [tab] class: 的行
  local methodMetaDataList=`echo "$metaData" | grep -E "f\tclass:"`
  
  # 利用数组实现字符串累加
  local methodSymbols=()
  
  # 按行(\n)迭代
  while read -r meta; do
    local methodName=`echo "$meta" | cut -f1`
    if [[ "$methodName" == "~"* ]]; then # 不需要 dtor
      continue
    fi
    
    local methodSignature=`echo "$meta" | cut -f3 | sed -E "s/\/\^  (.+){.+/\1/"`
    methodSymbols+=(${methodName}${METHOD_NAME_SIGNATURE_SEPARATOR}${methodSignature}${METHOD_SEPARATOR})
  done <<< "$methodMetaDataList"

  echo "${methodSymbols[@]}"
}

# $1: methodSymbols
# $2: gtestClassName
# $3: mockClassName
_gene_test_method_stub(){
  local methodSymbols=$1
  local gtestClassName=$2
  local mockClassName=$3

  # 生成方法 stub，默认使用 Value-Parameterized Tests
  # Multi Line Text
  # see http://www.guguncube.com/2140/unix-set-a-multi-line-text-to-a-string-variable-or-file-in-bash
  local stubTemplate="
  // test for: __method_signature__
  TEST_P(${gtestClassName}, __method_name__) {
    using namespace std;

    const ParamType& testParam = GetParam();

    // 默认模式 Mock，没有被 EXPECT_CALL 的方法会打印 warning，请按测试需要修改
    $mockClassName mock;
    // 配置/使用 mock
  }
  "
  # ParamType 在 _gene_test_fixture_class_stub() 中引入
  # $mockClassName 在 _gene_mock_stub() 中引入

  # 利用数组实现字符串累加
  local methodStubs=()

  while IFS="$METHOD_SEPARATOR" read -ra tmp; do
    for symbol in "${tmp[@]}"; do
      local methodName=`echo "$symbol" | cut -d"$METHOD_NAME_SIGNATURE_SEPARATOR" -f1`
      local methodSignature=`echo "$symbol" | cut -d"$METHOD_NAME_SIGNATURE_SEPARATOR" -f2`

      # 模板替换，生成 method stub
      local methodStub="${stubTemplate/__method_name__/$methodName}"
      methodStub=("${methodStub/__method_signature__/$methodSignature}")
      methodStubs+=$methodStub
    done
  done <<< "$methodSymbols"

  # 输出整个数组
  echo "
    /////////////////////////
    ////// Unit Test  ///////
    /////////////////////////

    ${methodStubs[@]}
  "
}

# $1: gtestClassName
# $2: origClassName
_gene_test_fixture_class_stub(){
  local gtestClassName=$1
  local origClassName=$2

  echo "
    ///////////////////////////
    ////// Test Fixture ///////
    ///////////////////////////

    class ${gtestClassName} : public ::testing::TestWithParam<std::string> {
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

      ${gtestClassName}() {
        using namespace std;
        const ParamType testParam = GetParam();
        //
      }

      ~${gtestClassName}() {
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
    ParamType valueParams[] = {\"foo\"};    // 请按测试需要修改 测试参数

    INSTANTIATE_TEST_CASE_P(${origClassName}_UnitTest, ${gtestClassName}, ::testing::ValuesIn(valueParams));
  "
}

_gene_assertions_stub(){
  echo "
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
        return ::testing::AssertionSuccess() << n << \" is even\";
      } else {
        return ::testing::AssertionFailure() << n << \" is odd\";
      }
    };
  "
}

_gene_matcher_stub(){
  echo "
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
     *   EXPECT_THAT(\"hello_world\", StartsWith(\"hello\"));
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
  "
}

# $1: methodSymbols
# $2: origClassName
# $3: mockClassName
_gene_mock_stub(){
  methodSymbols=$1
  origClassName=$2
  mockClassName=$3

  # 生成 ON_CALL 语句，格式见:
  #   https://github.com/abseil/googletest/blob/master/googlemock/docs/CookBook.md#delegating-calls-to-a-real-object
  local onCallTemplate="
    ON_CALL(*this, __method_name__(__on_call_method_param_sig__))
    .WillByDefault(Invoke(&_real, &$origClassName::__method_name__));\\n
  "
  local onCallStubs=()

  # 生成 MOCK_METHOD 语句，MOCK_METHOD 后面的数字是要 mock 的方法参数个数
  local mockMethodTemplate="
    MOCK_METHOD__method_param_count__(__method_name__, __method_signature__);\\n
  "
  local mockMethodStubs=()

  while IFS="$METHOD_SEPARATOR" read -ra tmp; do
    for symbol in "${tmp[@]}"; do
      local methodName=`echo "$symbol" | cut -d"$METHOD_NAME_SIGNATURE_SEPARATOR" -f1`
      local methodSignature=`echo "$symbol" | cut -d"$METHOD_NAME_SIGNATURE_SEPARATOR" -f2`
      local methodSignatureWithoutName="${methodSignature/$methodName/}"

      # 不要构造函数
      if [[ $methodSignatureWithoutName == "()"* ]]; then
        continue
      fi

      # 分析方法参数数量，看 "," 数量
      local paramCount=`echo $methodSignatureWithoutName | grep -c ','`
      if [[ $paramCount -eq 0 ]]; then # 如果没有 ","
        # 判断方法参数签名是否是 () 或 (void)
        local paramSignature=`echo $methodSignatureWithoutName | sed -E "s/.*\((.*)\)/\1/"`
        paramSignature=${paramSignature/' '/}

        if [[ $paramSignature == "void" || -z $paramSignature ]]; then
          paramCount=0 # 如果是，就没有参数
        else
          paramCount=1 # 否则就是 1 个参数
        fi
      else # 有 ","，参数数量为 个数 + 1
        paramCount=$(( $paramCount + 1))
      fi

      # 模板替换，生成 stub

      local onCallMethodParamSig=("")
      for (( count = 0; count < $paramCount; count++ )); do
        onCallMethodParamSig+=("_,")
      done
      onCallMethodParamSig="${onCallMethodParamSig[@]}" # 数组元素拼接成字符串
      onCallMethodParamSig=${onCallMethodParamSig%?} # 去掉最后一个 ","

      local onCallStub="${onCallTemplate//__method_name__/$methodName}"
      onCallStub="${onCallStub/__on_call_method_param_sig__/${onCallMethodParamSig}}"
      onCallStubs+=($onCallStub)

      local mockMethodStub="${mockMethodTemplate/__method_name__/$methodName}"
      mockMethodStub="${mockMethodStub/__method_signature__/$methodSignatureWithoutName}"
      mockMethodStub="${mockMethodStub/__method_param_count__/$paramCount}"
      mockMethodStubs+=($mockMethodStub)
    done
  done <<< "$methodSymbols"

  echo "
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
        ${onCallStubs[@]}
      }

      // 需要拿 Mock 封装一层的方法，这些方法会被 Mock 检验调用状态
      ${mockMethodStubs[@]}

     private:
      // 初始化真正的对象
      ${origClassName} _real;
    };
  "
}

# $1: origSrcFilePath
# $2: test Fixture stub code
# $3: assertions stub code
# $4: matcher stub code
# $5: matcher stub code
# $6: test method stub code
_gene_file_stub(){
  local origSrcFilePath=$1
  local matcherStubCode=$2
  local assertionsStubCode=$3
  local mockStubCode=$4
  local testFixtureStubCode=$5
  local testMethodStubCode=$6

  # 生成 测试文件的 stub
  # TestWithParam 泛型类型默认为 string
  echo "
    #include \"$origSrcFilePath\"
    #include \"gtest/gtest.h\"
    #include \"gmock/gmock.h\"

    using namespace std;

    namespace {

      $matcherStubCode

      $assertionsStubCode

      $mockStubCode

      $testFixtureStubCode

      $testMethodStubCode

    } // namespace
  "
}

# $1: unitTestFilePath
_createTestFile(){
  local unitTestFilePath=$1
  mkdir -p `dirname $unitTestFilePath`
  touch $unitTestFilePath
  chmod +w $unitTestFilePath
}

# $1: origSrcFilePath
# $2: fileExt
# $3: fileName
# $4: unitTestFilePath
_do_gene_stub_code_to_file(){
  local origSrcFilePath=$1
  local fileExt=$2
  local fileName=$3
  local unitTestFilePath=$4

  _createTestFile $unitTestFilePath

  local methodSymbols=`_parse_all_function_symbols_of_class $origSrcFilePath`

  # 良好的 C++ 源码，文件名就是类名
  local origClassName=${fileName/$fileExt/}
  # 类名+Test 就是对应的测试类
  local gtestClassName=${origClassName}Test
  # 类名+Mock 就是对应的 Mock 类
  local mockClassName=${origClassName}Mock

  local TEST_FIXTURE_STUB=`_gene_test_fixture_class_stub $gtestClassName $origClassName`
  local ASSERTIONS_STUB=`_gene_assertions_stub`
  local MATCHER_STUB=`_gene_matcher_stub`
  local MOCK_STUB=`_gene_mock_stub "$methodSymbols" $origClassName $mockClassName`
  local TEST_METHOD_STUB=`_gene_test_method_stub "$methodSymbols" $gtestClassName $mockClassName`

  local TEST_FILE_STUB=`_gene_file_stub $origSrcFilePath \
    "$MATCHER_STUB" \
    "$ASSERTIONS_STUB" \
    "$MOCK_STUB" \
    "$TEST_FIXTURE_STUB" \
    "$TEST_METHOD_STUB"
  `

  echo "$TEST_FILE_STUB" > $unitTestFilePath
  green "GTest file is generated: $unitTestFilePath"
}

###########
## public
###########

# $1: 需要测试的文件路径
gene_gtest_stub(){
  local origSrcFilePath=${1:?Provide a file path to gene gtest stub file}

  # 单元测试文件:
  # 1. 统一在 test 目录下，和 源文件 有同样的目录结果
  # 2. 命名规则：源代码文件名 + _unittest 后缀，扩展名为 cc
  local fileExt=.${origSrcFilePath#*.}
  local fileName=`basename $origSrcFilePath`
  local unitTestFilePath=test/${origSrcFilePath/$fileExt/}_unittest.cc

  if [[ ! -f $origSrcFilePath ]]; then
    red "[Error] Abort. File is NOT exist: $origSrcFilePath"
    return
  fi

  if [[ ! -f $unitTestFilePath ]]; then
    red "[Error] Abort. GTest file is aleady exist: $unitTestFilePath"
    return
  fi

  # gene gtest file at the test path
  _do_gene_stub_code_to_file $origSrcFilePath $fileExt $fileName $unitTestFilePath
}

$@
