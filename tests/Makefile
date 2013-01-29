#!/usr/bin/make -f

CFILES   := $(wildcard *.c)
OBJECTS  := $(CFILES:.c=.o)

CFLAGS += -O2 -std=gnu99
CFLAGS += -Isrc
CFLAGS += -Wall -Wextra -pedantic


.SUFFIXES: .c .h .o
.PHONY: all clear clean

all: ${OBJECTS}
	@printf "\033[1;32m LINK and run tests:\033[0m\n"
	@${CC} ${LDFLAGS} -o run_tests ${OBJECTS}
	@./run_tests
	@rm run_tests

%.o: %.c %.h
	@printf "\033[1m   CC\033[0m $< -> $@\n"
	@${CC} ${CFLAGS} -c -o $@ $<

%.o: %.c
	@printf "\033[1m   CC\033[0m $< -> $@\n"
	@${CC} ${CFLAGS} -c -o $@ $<

clean:
	@printf "\033[1;31m   RM\033[0m ${OBJECTS}\n"
	@rm -f ${OBJECTS}
