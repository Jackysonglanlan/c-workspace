# Using µnit is very simple; just include the header and add the C
# file to your sources.  That said, here is a simple Makefile to build
# the example.

CSTD:=99
OPENMP:=n
ASAN:=n
UBSAN:=n
EXTENSION:=
TEST_ENV:=
CFLAGS:=
AGGRESSIVE_WARNINGS=n

LIBDIR = ../../libs/munit

CFLAGS += -I$(LIBDIR)

ifeq ($(CC),pgcc)
        CFLAGS+=-c$(CSTD)
else
        CFLAGS+=-std=c$(CSTD)
endif

ifeq ($(OPENMP),y)
        ifeq ($(CC),pgcc)
                CFLAGS+=-mp
        else
                CFLAGS+=-fopenmp
        endif
endif

ifneq ($(SANITIZER),)
        CFLAGS+=-fsanitize=$(SANITIZER)
endif

ifneq ($(CC),pgcc)
        ifeq ($(EXTRA_WARNINGS),y)
                CFLAGS+=-Wall -Wextra -Werror
        endif

        ifeq ($(ASAN),y)
                CFLAGS+=-fsanitize=address
        endif

        ifeq ($(UBSAN),y)
                CFLAGS+=-fsanitize=undefined
        endif
endif

# tasks

all: test clean

test: example$(EXTENSION)
	$(TEST_ENV) ./example$(EXTENSION)

example$(EXTENSION): $(LIBDIR)/munit.h $(LIBDIR)/munit.c example.c
	$(CC) $(CFLAGS) -o $@ $(LIBDIR)/munit.c example.c

clean:
	rm -f example$(EXTENSION)

