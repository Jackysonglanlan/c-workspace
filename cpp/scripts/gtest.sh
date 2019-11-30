# !/bin/sh
#
#

### 备忘: bash 字符串操作
# FILE="example.tar.gz"
# echo "${FILE%%.*}" # example
# echo "${FILE%.*}"  # example.tar
# echo "${FILE#*.}"  # tar.gz
# echo "${FILE##*.}" # gz

#########
# 单元测试脚本
#
# 运行方式:
#   ./run.sh
#
# 基于 Google GTest 和 GMock
# see https://github.com/google/googletest/tree/master/googletest
#     https://github.com/google/googletest/blob/master/googletest/docs/advanced.md
#     https://github.com/google/googletest/blob/master/googlemock/docs/CookBook.md
#     https://github.com/abseil/googletest/blob/master/googlemock/docs/CheatSheet.md
#
# 如何编写单元测试文件: https://github.com/google/googletest/blob/master/googletest/docs/primer.md
#########

set -euo pipefail
trap "echo 'error: Script failed: see failed command above'" ERR

# 根目录绝对路径
ROOT_DIR="${BASH_SOURCE%/*}/.."
if [[ ! -d "$ROOT_DIR" ]]; then ROOT_DIR="$PWD"; fi

TEST_DIR=$ROOT_DIR/test

source $ROOT_DIR/scripts/utils.sh

use_red_green_echo "Test"

# mac 下可以用 clang++
CC=g++

# morden c++
CFLAGS='--std=c++14'

# GTest GMock
GTEST_DIR=$TEST_DIR/libs/gtest
GMOCK_DIR=$TEST_DIR/libs/gmock
GMOCK_MAIN_FILE=$GMOCK_DIR/src/gmock_main.cc
GMOCK_LIB_FILE=$TEST_DIR/libgmock.a

# 需要测试的源文件，可以是 .h .cpp .cc，或者直接是 GTest 单元测试文件(_unittest.cc)
#
# 脚本会自动查询对应的 GTest 单元测试文件，测试文件命名约定如下:
# 1. 统一在 test 目录下，和 源文件 有同样的目录结果
# 2. 命名规则：源代码文件名 + _unittest 后缀，扩展名为 cc
#
# E.g:
#   foo/bar.h -> test/foo/bar_unittest.cc
#   foo/bar.cpp -> test/foo/bar_unittest.cc
#   foo/bar.cc -> test/foo/bar_unittest.cc
#   test/foo/bar_unittest.cc -> test/foo/bar_unittest.cc # 直接写测试文件
SRC_FILES=(utils/dispatcher/Dispatcher.h utils/pipe/Pipe.h utils/combinator/Combinator.h utils/Demo.cpp
  # utils/debug/Debug.h
)

###########
## private
###########

# 自动生成单元测试文件名
#
# $1: src file array
_gene_unit_test_file_names(){
  local unitTestSuffix="_unittest.cc"
  local UNIT_TEST_FILES=()
  for src in "$@"
  do
    # 文件路径错误
    if [[ ! -f "$ROOT_DIR/$src" ]]; then
      red "Abort! File NOT exist: $src"
      exit
    fi
    
    if [[ $src == "test/"* ]]; then
      src=${src/test\//""}
    fi
    
    local testFilePath=""
    if [[ $src == *$unitTestSuffix  ]]; then # 直接写的测试文件
      testFilePath=$src
    else
      testFilePath=${src%%.*}_unittest.cc
    fi
    
    UNIT_TEST_FILES+=($TEST_DIR/$testFilePath)
  done
  echo "${UNIT_TEST_FILES[@]}"
}

_clean(){
  (cd $ROOT_DIR && make clean_without_so)
}

_build_GMock(){
  if [[ -f $GMOCK_LIB_FILE ]]; then
    green "GMock lib file $GMOCK_LIB_FILE already exist..."
    return
  fi
  
  yellow "Start building GMock..."
  $CC $CFLAGS \
  -isystem ${GTEST_DIR}/include -I${GTEST_DIR} \
  -isystem ${GMOCK_DIR}/include -I${GMOCK_DIR} \
  -pthread -c ${GTEST_DIR}/src/gtest-all.cc ${GMOCK_DIR}/src/gmock-all.cc
  
  ar -rv $GMOCK_LIB_FILE gtest-all.o gmock-all.o
  green "GMock is built: $GMOCK_LIB_FILE"
  
  _clean
}

_gene_dep_headers_compile_args(){
  # -I 指定 #include 需要的 .h 文件查找路径
  echo -I$ROOT_DIR -I${GTEST_DIR}/include -I${GMOCK_DIR}/include
}

# 遍历 dist 中的所有 .so 文件，生成对应的编译参数
# E.g:
# 假如 dist 中有 libfoo.so 和 libbar.so 文件，这个方法生成: -Ldist -lfoo -lbar
_gene_dep_libs_compile_args(){
  # -L 指定需要的 .so/.a 文件查找路径
  echo -L$ROOT_DIR/dist `find $ROOT_DIR/dist -name 'lib*.so' | xargs -I{} basename "{}" | sed 's/lib/-l/g' | sed 's/.so//g'`
}

# $1: unit test files to run
_compile_and_run_gtest(){
  # GMock 包含 GTest，所以只需要编译 GMock 即可
  _build_GMock
  
  # 编译依赖项
  (cd $ROOT_DIR && make so_dependencies)
  
  # 编译 gtest 执行文件
  local test_file=gtest.out
  
  yellow "Prepare compiling GTest executable file: $test_file"
  
  local libArgs=`_gene_dep_libs_compile_args`
  yellow "  lib args: $libArgs"
  
  local headerArgs=`_gene_dep_headers_compile_args`
  yellow "  header args: $headerArgs"
  
  local unit_test_files=$1
  yellow "Unit Test Files: $unit_test_files"
  
  local compile_command="
  $CC $CFLAGS -pthread
  $libArgs $headerArgs
  $unit_test_files
  $GMOCK_MAIN_FILE $GMOCK_LIB_FILE
  -o $ROOT_DIR/$test_file
  "

  yellow "Compile GTest exec file: $test_file"
  echo $compile_command

  $compile_command

  green "Done"

  # 因为依赖了 root 目录 Makefile 的 so_dependencies 任务创建的 so 文件，所以必须在 root 目录执行，否则
  # 会报错 Library not loaded: image not found
  (cd $ROOT_DIR && ./$test_file)

  _clean
}

###########
## public
###########

test_all(){
  local unit_test_files=`_gene_unit_test_file_names "${SRC_FILES[@]}"`
  _compile_and_run_gtest "$unit_test_files"
}

# 只编译一个 test 文件，用于快速运行一些代码
# $1: one gtest file path
test_one(){
  _compile_and_run_gtest "$1"
}

# $1: src file which you want to gtest
gene_gtest_stub_cpp(){
  yellow "Start generating GTest stub for $1..."

  local execFile="gene-gtest-stub.out"
  $CC $CFLAGS $ROOT_DIR/scripts/gene-gtest-stub.cpp -o $execFile

  ./$execFile "$ROOT_DIR/.tags" $1 || echo

  rm $execFile

  green "Done"
}

# scripts/gtest.sh test_one test/utils/Demo_unittest.cc
${@:-test_all}
