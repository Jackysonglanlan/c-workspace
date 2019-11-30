# Makefile
# See: https://seisman.github.io/how-to-write-makefile/overview.html

# Makefiel Functions
# See https://www.gnu.org/software/make/manual/html_node/Functions.html

# 编译C++程序:
# See: https://blog.csdn.net/liujiayu2/article/details/49864381
#

# mac 下可以用 clang++
CC=g++

# morden c++
# -I 为 .h 文件搜寻路径，当编译器在系统文件夹下找不到声明的 .h 时，就会从这里指定的路径寻找
CFLAGS=--std=c++17 -Isrc -Ilibs

# LD_LIBRARY_PATH is needed by linux, NOT -L
RUNTIME_FLAG=LD_LIBRARY_PATH=dist

# 自己写的依赖库，必须是 .h，且必须有对应的 .cpp，会被编译成 .so
DEPS=src/utils/io/io.h src/utils/common/str.h src/utils/debug/Debug.h

# demo.o 会匹配下面 %.o 规则
MAIN=demo.o

######## function ########

# Makefile 中不支持真正意义上的自定义函数
# 自定义函数的本质是多行变量
# 预定义的call函数在调用时将参数传递给多行变量
# 自定义函数是call函数的实参，并在call中被执行

# WARN: 这个方法由于调用了 shell，在项目文件过多的时候效率很低
# 遍历 $1 目录下的 .so，生成 $(CC) 需要的参数列表，以便链接到 demo.out
#
# Usage: $(call gene_so_lib_arg_list_from_dir,dist)
# Output: -Ldist -lXXX -lYYY -lZZZ，其中 -Ldist 代表 .so 的查找路径是 dist 目录
#
# $1: searching dir
define gene_so_lib_arg_list_from_dir
  -L$(1) `find $(1) -name 'lib*.so' | sed 's/$(1)\/lib/-l/g' | sed 's/.so//g'`
endef
# 上面注意：使用 `` 操作符并不会执行 find xxx，而是替换参数以后，返回新的命令，等到下面任务引用的使用才会真正调用

SO_LIB_ARGS := $(call gene_so_lib_arg_list_from_dir,dist)

DEPS_SO=$(DEPS:%.h=%.so)

# 测试用
# test:
# 	echo $(SO_LIB_ARGS)

######## rules ########

# 规则默认是基于文件的，所以 如果规则名和文件名相同，make 会检测这个文件是否是 up to date，如果是，就不执行了(实现增量编译)
# 注意，所有规则执行的命令都 必须 以 tab 开头!
# $@ 代表下面 ':' 的左边，$^ 代表 ':' 的右边，$< 代表 ':' 右边的列表一个一个的传递进来

.PHONY: all

# 如果只执行 make，默认从第一个规则开始，这个规则依赖 run 和 clean 规则，run 和 clean 都执行了，all 再执行自己(空，不做任何事)
all: static_check run clean

static_check:
	@echo start code static check...
	@mkdir -p test/reports
	@cpplint --output=junit **/*.cpp > test/reports/static-check-output.xml 2>&1 || echo ok

# run 依赖 demo 规则，当 demo 规则执行完毕以后，run 再执行自己，即 ./demo.so
run: demo
	@echo Running demo:
	@$(RUNTIME_FLAG) ./$^.out

# demo 依赖上面的 MAIN 定义的东西，这里是 demo.o，意思是 demo 依赖 demo.o 规则，我们没有 demo.o，但是有 %.o，可以匹配
demo: so_dependencies $(MAIN)
	@$(CC) $(MAIN) -o $@.out $(SO_LIB_ARGS) $(CFLAGS)

# $(DEPS_SO) 的值会匹配下面的 %.so 规则
so_dependencies: $(DEPS_SO)

# %.o 依赖 %.cpp，但没有定义 cpp 规则，所以它开始执行自己: g++ -c xxx.cpp -o xxx.o $(CFLAGS)
%.o: %.cpp
	@$(CC) -c $< -o $@ $(CFLAGS)

# TODO: 即使文件没有修改也会编译，没有增量编译，查找原因
# 编译 .cpp 成 libXXX.so，放在 dist 目录下
%.so: %.cpp
	@echo "Compile $@"
	@$(CC) -fpic -shared $< -o $(subst $(dir $@), dist/lib, $@) $(SO_LIB_ARGS) $(CFLAGS)

clean:
	@echo "Cleaning *.o, *.so files ..."
	@find . \( -name "*.so" -o -name "*.o" \) -type f -exec rm {} +
	@make clean_exec
	@echo "Done"

clean_exec:
	@echo "Cleaning exec files ..."
	@find . \( -name "*.out" \) -type f -exec rm {} +
	@echo "Done"

clean_without_so:
	@echo "Cleaning all without *.so ..."
	@find . \( -name "*.o" \) -type f -exec rm {} +
	@make clean_exec
	@echo "Done"

