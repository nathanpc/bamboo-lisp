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
	@echo "Linking tiny REPL into an executable..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILDDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/stamp:
	@echo "Creating directory for tiny REPL build files..."
	$(MKDIR) $(@D)
	$(TOUCH) $@

$(BAMBOOLIB):
	cd ../lisp && $(MAKE)

run: compile
	$(TARGET)

debug: CFLAGS += -g3 -DDEBUG
debug: compile
	@echo "Compiled tiny REPL for debug!"
	$(GDB) $(TARGET)

memcheck: CFLAGS += -g3 -DDEBUG -DMEMCHECK
memcheck: compile
	@echo "Compiled tiny REPL for memcheck!"
	valgrind --tool=memcheck --leak-check=yes --show-leak-kinds=all \
		--track-origins=yes --log-file=$(BUILDDIR)/valgrind.log $(TARGET)
	cat $(BUILDDIR)/valgrind.log

clean:
	@echo "Cleaning tiny REPL build files..."
	$(RM) $(OBJECTS)
	$(RM) $(TARGET)
