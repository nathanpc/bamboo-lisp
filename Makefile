### Makefile
### Automates the build and everything else of the project.
###
### Author: Nathan Campos <nathan@innoveworkshop.com>

# Project
PROJECT = bamboo
PLATFORM := $(shell uname -s)

# Tools
CC = gcc
RM = rm -f
GDB = gdb
MKDIR = mkdir -p
TOUCH = touch

# Directories and Paths
SRCDIR = src
REPLDIR = repl
BUILDDIR := build
EXAMPLEDIR := examples
TARGET = $(BUILDDIR)/$(PROJECT)

# Sources and Flags
SOURCES += $(SRCDIR)/bamboo.c
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.c=.o))
REPLSRC = $(REPLDIR)/main.c $(REPLDIR)/input.c $(REPLDIR)/functions.c
OBJECTS += $(patsubst $(REPLDIR)/%,$(BUILDDIR)/%,$(REPLSRC:.c=.o))
CFLAGS = -Wall -Wno-psabi -DUNICODE
LDFLAGS = 

.PHONY: all run test debug memcheck examples clean
all: $(BUILDDIR)/stamp $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: $(REPLDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/stamp:
	$(MKDIR) $(@D)
	$(TOUCH) $@

run: $(BUILDDIR)/stamp $(TARGET)
	$(TARGET)

debug: CFLAGS += -g3 # -DDEBUG
debug: clean $(BUILDDIR)/stamp $(TARGET)
	$(GDB) $(TARGET)

memcheck: CFLAGS += -g3 -DDEBUG -DMEMCHECK
memcheck: clean $(BUILDDIR)/stamp $(TARGET)
	valgrind --tool=memcheck --leak-check=yes --show-leak-kinds=all \
		--track-origins=yes --log-file=valgrind.log $(TARGET)
	cat valgrind.log

examples:
	cd $(EXAMPLEDIR) && $(MAKE)

clean:
	$(RM) -r $(BUILDDIR)
	$(RM) valgrind.log
	cd $(EXAMPLEDIR) && $(MAKE) clean
