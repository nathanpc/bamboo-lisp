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
BUILDDIR := build
TARGET = $(BUILDDIR)/$(PROJECT)

# Sources and Flags
SOURCES += $(SRCDIR)/main.c $(SRCDIR)/bamboo.c
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.c=.o))
CFLAGS = -Wall -DUNICODE
LDFLAGS = 

.PHONY: all run test debug memcheck clean
all: $(BUILDDIR)/stamp $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
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
	valgrind --tool=memcheck --leak-check=yes --show-leak-kinds=all --track-origins=yes --log-file=valgrind.log $(TARGET)
	cat valgrind.log

clean:
	$(RM) -r $(BUILDDIR)
	$(RM) valgrind.log
