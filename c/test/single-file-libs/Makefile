#####
# https://github.com/ennorehling/clibs
# https://github.com/DanielGibson/Snippets
#####


CU_TEST_FILES = obj/CuTest.o \
	obj/test-selist.o obj/selist.o \
	obj/test-critbit.o obj/critbit.o

MAIN_TEST_FILES = main/test-DG_misc.run \
	main/test-DG_dynarr.run \
	main/test-tinytime.run \
	main/test-heap-permutation.run

LIBDIR = ../../libs/single-file-libs

CFLAGS += -g -O3 -Wall -Wextra -Wstrict-aliasing=2 -Wno-unused-function -I$(LIBDIR)

ifeq "$(CC)" "clang"
CFLAGS += -Weverything -Wno-padded
# stop clang warning us about strncmp/vsprintf:
CFLAGS += -Wno-disabled-macro-expansion -Wno-format-nonliteral
endif

all: test clean

test: bin/tests main/tests
	@bin/tests
	@$(foreach e, $(MAIN_TEST_FILES), $e;) # main file can be executed directly.

main bin obj:
	# $@ is the target's name
	mkdir -p $@

clean:
	@rm -rf bin obj main

#### CuTest-based tests

bin/tests: tests.c $(CU_TEST_FILES) | bin # invokes 'bin' target, not the 'bin' file
	# $^ is the dependencies
	$(CC) $(CFLAGS) -lm $^ -o $@ # output file will be: bin/tests

obj/%.o: $(LIBDIR)/%.c $(LIBDIR)/%.h | obj # invokes 'obj' target
	# $< is the file name of .c
	$(CC) -o $@ -c $< $(CFLAGS)

obj/test-%.o: test-%.c | obj
	$(CC) -o $@ -c $< $(CFLAGS)

#### main based tests

main/tests: $(MAIN_TEST_FILES)

main/%.run: %.c | main
	$(CC) $< -o $@ -I$(LIBDIR)
