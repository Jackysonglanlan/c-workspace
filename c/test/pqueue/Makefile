#####
# https://github.com/vy/libpqueue
#####

all: run clean
run: compile
	./pqueue.run
	./pqueue-multiattr.run
compile:
	cc -Wall -g ../../libs/pqueue/pqueue.c sample.c -o pqueue.run
	cc -Wall -g ../../libs/pqueue/pqueue.c sample-multiattr.c -o pqueue-multiattr.run
clean:
	rm -f *.run
	rm -rf *.dSYM
.PHONY: all run clean

