#!/usr/bin/make -f

#PROJNAME := $(notdir $(PWD))
PROJNAME := "testc"
CFILES   := $(wildcard src/*.c)
OBJECTS  := $(CFILES:.c=.o)
CPLUGINS := $(wildcard src/plugins/*.c)
OPLUGINS := $(CPLUGINS:.c=.so)
PREFIX   ?= "/usr/local"

_ver0   := $(shell git describe HEAD 2>/dev/null)
_ver1   := $(shell git describe --tags HEAD 2>/dev/null)
_ver2   := $(shell git rev-parse --verify --short HEAD 2>/dev/null)
VERSION := $(or $(_ver0),$(_ver1),$(_ver2))

CFLAGS += -O2 -std=gnu99
CFLAGS += -Isrc
# CFLAGS += -Wall -Wextra -pedantic
CFLAGS += -DVERSION=\"$(VERSION)\"
#LDFLAGS += -L`gcc -print-file-name=` -lc -lgcc


.SUFFIXES: .c .h .o .so
.PHONY: all clear clean install uninstall

all: $(PROJNAME)

debug: clean
	@CFLAGS+=-g make

$(PROJNAME): $(OBJECTS) $(OPLUGINS)
	@echo "\033[1;32m LINK\033[0m \033[1m${PROJNAME}\033[0m <- $(OBJECTS)"
	@$(CC) $(LDFLAGS) -o ${PROJNAME} $(OBJECTS)

%.o: %.c %.h
	@echo "\033[1m   CC $<\033[0m -> $@"
	@$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.c
	@echo "\033[1m   CC $<\033[0m -> $@"
	@$(CC) $(CFLAGS) -c -o $@ $<

%.so: %.c
	@echo "\033[1m   CC $<\033[0m -> $@"
	@$(CC) $(CFLAGS) -o $@ $< -shared

clean:
	@echo "\033[1;31m   RM\033[0m" $(OBJECTS) $(OPLUGINS)
	@rm -f $(OBJECTS) $(OPLUGINS)

clear: clean
	@echo "\033[1;31m   RM\033[0m" "${PROJNAME}"
	@rm -f ${PROJNAME}

install: all
	install -m 755 ${PROJNAME} "$(PREFIX)/bin/"

uninstall: all
	rm "$(PREFIX)/bin/$(PROJNAME)"
