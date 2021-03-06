#!/usr/bin/make -f

#PROJNAME := $(notdir $(PWD))
PROJNAME := "tmux-sb"
CFILES   := $(wildcard src/*.c)
OBJECTS  := $(CFILES:.c=.o)
PREFIX   ?= "/usr/local"

_ver0   := $(shell git describe HEAD 2>/dev/null)
_ver1   := $(shell git describe --tags HEAD 2>/dev/null)
_ver2   := $(shell git rev-parse --verify --short HEAD 2>/dev/null)
VERSION := $(or $(_ver0),$(_ver1),$(_ver2))

CFLAGS += -O2 -std=gnu99
CFLAGS += -Isrc
CFLAGS += -Wall -Wextra -pedantic
CFLAGS += -DVERSION=\"$(VERSION)\"
#LDFLAGS += -L`gcc -print-file-name=` -lc -lgcc


.SUFFIXES: .c .h .o
.PHONY: all clear clean install uninstall

all: $(PROJNAME)

debug: clean
	@CFLAGS+=-g make

$(PROJNAME): $(OBJECTS)
	@echo -e "\033[1;32m LINK\033[0m ${PROJNAME}"
	@$(CC) $(LDFLAGS) -o ${PROJNAME} $(OBJECTS)

%.o: %.c %.h
	@echo -e "\033[1m   CC\033[0m $< -> $@"
	@$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo -e "\033[1;31m   RM\033[0m" $(OBJECTS)
	@rm -f $(OBJECTS)

clear: clean
	@echo -e "\033[1;31m   RM\033[0m" "${PROJNAME}"
	@rm -f ${PROJNAME}

install: all
	install -m 755 ${PROJNAME} "$(PREFIX)/bin/"

uninstall: all
	rm "$(PREFIX)/bin/$(PROJNAME)"
