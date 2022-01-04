### Makefile
### Automates the build and everything else of the project.
###
### Author: Nathan Campos <nathan@innoveworkshop.com>

# Tools
CC = gcc
CXX = g++
RM = rm -f
GDB = gdb
MKDIR = mkdir -p
TOUCH = touch

# Directories and Paths
SRCDIR = src
REPLDIR = repl
BUILDDIR := build
EXAMPLEDIR := examples

# Sources and Flags
SOURCES += $(SRCDIR)/bamboo.c $(SRCDIR)/BambooWrapper.cpp
OBJECTS := $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(SOURCES))
OBJECTS := $(patsubst $(SRCDIR)/%.cpp, $(BUILDDIR)/%.o, $(OBJECTS))
CFLAGS = -Wall -Wno-psabi -DUNICODE
LDFLAGS = 

.PHONY: all compile run test debug memcheck repl examples clean
all: compile repl

compile: $(BUILDDIR)/stamp $(OBJECTS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)/stamp:
	$(MKDIR) $(@D)
	$(TOUCH) $@

run: compile
	cd $(REPLDIR) && $(MAKE) run

debug: CFLAGS += -g3 -DDEBUG
debug: clean compile
	cd $(REPLDIR) && $(MAKE) debug

memcheck: CFLAGS += -g3 -DDEBUG -DMEMCHECK
memcheck: clean compile
	cd $(REPLDIR) && $(MAKE) memcheck

repl: compile
	cd $(REPLDIR) && $(MAKE)

examples: compile
	cd $(EXAMPLEDIR) && $(MAKE)

clean:
	$(RM) -r $(BUILDDIR)
	$(RM) valgrind.log
	cd $(REPLDIR) && $(MAKE) clean
	cd $(EXAMPLEDIR) && $(MAKE) clean
