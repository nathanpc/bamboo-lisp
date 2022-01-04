### Makefile
### Automates the build of the Bamboo REPL.
###
### Author: Nathan Campos <nathan@innoveworkshop.com>

# Project
PROJECT = bamboo
PLATFORM := $(shell uname -s)

# Tools
CC = gcc
CXX = g++
RM = rm -f
GDB = gdb
MKDIR = mkdir -p
TOUCH = touch

# Directories and Paths
BUILDDIR := ../build
TARGET = $(BUILDDIR)/$(PROJECT)

# Sources and Flags
PREREQS = $(BUILDDIR)/bamboo.o
SOURCES = main.c input.c functions.c
OBJECTS := $(patsubst %.c, $(BUILDDIR)/%.o, $(SOURCES))
CFLAGS = -Wall -Wno-psabi -DUNICODE
LDFLAGS = 

.PHONY: all compile run debug memcheck clean
all: $(TARGET)

compile: $(TARGET)

$(TARGET): $(OBJECTS) $(PREREQS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(BUILDDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: 
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
