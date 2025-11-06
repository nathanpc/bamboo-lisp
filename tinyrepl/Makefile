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

.PHONY: all compile run debug gdb memcheck clean
all: compile

compile: $(BUILDDIR)/stamp $(TARGET)

$(TARGET): $(OBJECTS) $(BAMBOOLIB)
	@echo "$(COLOR_GREEN)Linking tiny REPL into an executable...$(COLOR_END)"
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILDDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/stamp:
	@echo "$(COLOR_BLUE)Creating directory for tiny REPL build files...$(COLOR_END)"
	$(MKDIR) $(@D)
	$(TOUCH) $@

$(BAMBOOLIB):
	cd ../lisp && $(MAKE)

run: compile
	$(TARGET)

debug: CFLAGS += -g3 -DDEBUG
debug: compile
	@echo "$(COLOR_YELLOW)Compiled tiny REPL for debug!$(COLOR_END)"

gdb: debug
	@echo "$(COLOR_GREEN)Running tiny REPL via GDB...$(COLOR_END)"
	$(GDB) $(TARGET)

memcheck: CFLAGS += -g3 -DDEBUG -DMEMCHECK
memcheck: compile
	@echo "$(COLOR_YELLOW)Compiled tiny REPL for memcheck!$(COLOR_END)"
	valgrind --tool=memcheck --leak-check=yes --show-leak-kinds=all \
		--track-origins=yes --log-file=$(BUILDDIR)/valgrind.log $(TARGET)
	cat $(BUILDDIR)/valgrind.log

clean:
	@echo "$(COLOR_GREEN)Cleaning tiny REPL build files...$(COLOR_END)"
	$(RM) $(OBJECTS)
	$(RM) $(TARGET)
