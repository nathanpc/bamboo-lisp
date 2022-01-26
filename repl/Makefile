### Makefile
### Automates the build of the Bamboo REPL.
###
### Author: Nathan Campos <nathan@innoveworkshop.com>

include ../variables.mk

# Project
PROJECT = bamboo

# Directories and Paths
BUILDDIR := ../build
TARGET = $(BUILDDIR)/$(PROJECT)

# Sources and Flags
PREREQS = $(BUILDDIR)/bamboo.o
SOURCES = main.c input.c functions.c strutils.c fileutils.c
OBJECTS := $(patsubst %.c, $(BUILDDIR)/%.o, $(SOURCES))

.PHONY: all compile run debug memcheck clean
all: $(TARGET)

compile: $(TARGET)

$(TARGET): $(OBJECTS) $(PREREQS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILDDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	$(TARGET)

debug: CFLAGS += -g3 -DDEBUG
debug: clean $(TARGET)
	$(GDB) $(TARGET)

memcheck: CFLAGS += -g3 -DDEBUG -DMEMCHECK
memcheck: clean $(TARGET)
	valgrind --tool=memcheck --leak-check=yes --show-leak-kinds=all \
		--track-origins=yes --log-file=valgrind.log $(TARGET)
	cat valgrind.log

clean:
	$(RM) $(TARGET)
