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
ifdef USE_PLOTTING
	SOURCES += plotting/gnuplot.c
endif
OBJECTS := $(patsubst %.c, $(BUILDDIR)/%.o, $(SOURCES))

.PHONY: all compile run debug memcheck clean
all: compile

compile: $(BUILDDIR)/plotting/stamp $(TARGET) 

$(TARGET): $(OBJECTS) $(PREREQS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILDDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/plotting/stamp:
	$(MKDIR) $(@D)
	$(TOUCH) $@

run: $(TARGET)
	$(TARGET) -l ../corelib/core.bam

debug: CFLAGS += -g3 -DDEBUG
debug: clean $(TARGET)
	$(GDB) $(TARGET)

memcheck: CFLAGS += -g3 -DDEBUG -DMEMCHECK
memcheck: clean $(TARGET)
	valgrind --tool=memcheck --leak-check=yes --show-leak-kinds=all \
		--track-origins=yes --log-file=$(BUILDDIR)/valgrind.log $(TARGET)
	cat $(BUILDDIR)/valgrind.log

clean:
	$(RM) $(TARGET)
