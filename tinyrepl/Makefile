### Makefile
### Automates the build of the tiny REPL for testing.
###
### Author: Nathan Campos <nathan@innoveworkshop.com>

include ../variables.mk

# Project
PROJECT := tinyrepl

# Directories and Paths
BUILDDIR  := ../build/$(PROJECT)
TARGET    := $(BUILDDIR)/$(PROJECT)
BAMBOOLIB := ../build/lisp/libbamboo.a

# Sources and Flags
SOURCES := repl.c
OBJECTS := $(patsubst %.c, $(BUILDDIR)/%.o, $(SOURCES))

.PHONY: all compile run debug memcheck clean
all: compile

compile: $(BUILDDIR)/stamp $(TARGET)

$(TARGET): $(OBJECTS) $(BAMBOOLIB)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILDDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/stamp:
	$(MKDIR) $(@D)
	$(TOUCH) $@

$(BAMBOOLIB):
	cd ../lisp && $(MAKE)

run: compile
	$(TARGET)

debug: CFLAGS += -g3 -DDEBUG
debug: compile
	$(GDB) $(TARGET)

memcheck: CFLAGS += -g3 -DDEBUG -DMEMCHECK
memcheck: compile
	valgrind --tool=memcheck --leak-check=yes --show-leak-kinds=all \
		--track-origins=yes --log-file=$(BUILDDIR)/valgrind.log $(TARGET)
	cat $(BUILDDIR)/valgrind.log

clean:
	$(RM) $(OBJECTS)
	$(RM) $(TARGET)
