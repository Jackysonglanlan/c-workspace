#!/bin/sh
#

set -euo pipefail
trap "echo 'error: Script failed: see failed command above'" ERR

ROOT_DIR=$PWD

. "$ROOT_DIR"/scripts/utils.sh

use_red_green_echo "C++"

###########
## private
###########

# cscope see: https://docs.oracle.com/cd/E19205-01/819-5265/bjanq/index.html
#
# Generate a list of all source files starting from the current directory
_cscope_build() {
  # The -o means logical OR
  find . -name "*.c" -o -name "*.cc" -o -name "*.cpp" -o -name "*.h" -o -name "*.hh" -o -name "*.hpp" > cscope.files
  # -q build fast but larger database
  # -R search symbols recursively
  # -b build the database only, don't fire cscope
  # -i file that contains list of file paths to be processed
  # This will generate a few cscope.* files
  cscope -q -R -b -i cscope.files
  # Temporary files, remove them
  # rm -f cscope.files cscope.in.out cscope.po.out
  echo "The cscope database is generated"
}

_ctag_build(){
  ctags -R -f .tags
  green "The .ctags file is generated"
}

###########
## public
###########

##### run #####

run_in_mac(){
  yellow --------- mac ---------
  make clean && make || make clean
  _cscope_build
  _ctag_build
  yellow --------- mac ---------
}

# 需要 jacky/cpp:sdk image，见 Docker repo 的 dev-sdks/cpp 目录
run_in_docker(){
  yellow --------- linux ---------
  docker exec dev-sdks_cpp_run_1 g++ -v
  echo
  green Running...
  docker cp . dev-sdks_cpp_run_1:/home/cpp/
  docker exec -w=/home/cpp dev-sdks_cpp_run_1 make clean && make || make clean
  yellow --------- linux ---------
}

##### gtest #####

gtest(){
  yellow "Start running GTest..."
  scripts/gtest.sh
}

playground(){
  yellow "Start Play ground..."
  make clean && make || make clean
  green "Done"
  # scripts/gtest.sh test_one test/utils/Demo_unittest.cc
}

##### gtest generate #####

gene_gtest_stub_cpp(){
  local srcFile="$1"
  ./scripts/gtest.sh gene_gtest_stub_cpp "$srcFile"
}

# 同 ./gtest-stub-code-generator.sh gene_gtest_stub，生成 GTest 测试文件 stub
gene_gtest_stub(){
  "$ROOT_DIR"/scripts/gene-gtest-stub.sh gene_gtest_stub "$@"
}

# default use "playground"
"${@:-playground}"

