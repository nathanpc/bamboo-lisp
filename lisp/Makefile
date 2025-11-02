### Makefile
### Automates the build of the Bamboo Lisp library.
###
### Author: Nathan Campos <nathan@innoveworkshop.com>

include ../variables.mk

# Directories and Paths
PROJECT  := libbamboo
BUILDDIR := ../build/lisp
TARGET   := $(BUILDDIR)/$(PROJECT).a

# Sources and Flags
SOURCES := bamboo.c
OBJECTS := $(patsubst %.c, $(BUILDDIR)/%.o, $(SOURCES))

.PHONY: all compile run debug memcheck clean
all: compile

compile: $(BUILDDIR)/stamp $(TARGET)

$(TARGET): $(OBJECTS)
	@echo "$(COLOR_GREEN)Linking Lisp interpreter static library...$(COLOR_END)"
	$(AR) -rcs $@ $^

$(BUILDDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/stamp:
	@echo "$(COLOR_BLUE)Creating directory for Lisp interpreter build files...$(COLOR_END)"
	$(MKDIR) $(@D)
	$(TOUCH) $@

run: compile

debug: CFLAGS += -g3 -DDEBUG
debug: compile
	@echo "$(COLOR_YELLOW)Compiled Lisp interpreter for debug!$(COLOR_END)"

memcheck: CFLAGS += -g3 -DDEBUG -DMEMCHECK
memcheck: compile
	@echo "$(COLOR_YELLOW)Compiled Lisp interpreter for memcheck!$(COLOR_END)"

clean:
	@echo "$(COLOR_GREEN)Cleaning Lisp interpreter build files...$(COLOR_END)"
	$(RM) $(OBJECTS)
	$(RM) $(TARGET)
