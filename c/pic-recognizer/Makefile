
CC=cc

CFLAGS=

MAIN_FILE=demo.c
OUTPUT=$(MAIN_FILE:%.c=%.out) # .out is git ignored

all: run clean

run: compile
	./$(OUTPUT)

compile:
	$(CC) -o $(OUTPUT) $(MAIN_FILE) $(CFLAGS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f {**/*,*}.{out,so,o}

.PHONY: all run clean

