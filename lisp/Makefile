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
	$(AR) -rcs $@ $^

$(BUILDDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/stamp:
	$(MKDIR) $(@D)
	$(TOUCH) $@

run: compile

debug: CFLAGS += -g3 -DDEBUG
debug: compile

memcheck: CFLAGS += -g3 -DDEBUG -DMEMCHECK
memcheck: compile

clean:
	$(RM) $(OBJECTS)
	$(RM) $(TARGET)
