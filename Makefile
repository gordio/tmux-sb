#!/usr/bin/make -f

PROJNAME := $(notdir $(PWD))
CFILES := $(wildcard src/*.c)
OBJECTS = $(CFILES:.c=.o)
PREFIX = /usr/local

PCFLAGS = -O2
PCFLAGS += $(CFLAGS)

.SUFFIXES: .a .c .h .o
.PHONY: all static clear clean install uninstall types

all: $(OBJECTS)
	@echo -e "\033[1;32m LINK\033[0m" $(PROJNAME)
	@$(CC) $(OBJECTS) -o "$(PROJNAME)"

%.o: %.c
	@echo -e "\033[1m   CC\033[0m" $<
	@$(CC) $(PCFLAGS) -c $< -o $@

clear:
	@echo -e "\033[1;31m   RM\033[0m" $(OBJECTS)
	@rm -f $(OBJECTS)

clean: clear
	@echo -e "\033[1;31m   RM\033[0m" "$(PROJNAME)"
	@rm -f "$(PROJNAME)"

install: all
	install -m 744 "$(PROJNAME)" "$(PREFIX)/bin/"

uninstall: all
	rm "$(PREFIX)/bin/$(PROJNAME)"

types: types.vim
types.vim: src/*.[ch]
	@echo -e "\033[1m MAKE\033[0m" types.vim
	@exuberant-ctags --c-kinds=gstu -o- src/*.[ch] |\
		awk 'BEGIN{echo -e("syntax keyword Type\t")}\
			{echo -e("%s ", $$1)}END{print ""}' > $@

