### variables.mk
### Common variables used throughout the project.
###
### Author: Nathan Campos <nathan@innoveworkshop.com>

# Environment
PLATFORM := $(shell uname -s)

# Fancy colors.
COLOR_RED    := \033[0;31m
COLOR_GREEN  := \033[0;32m
COLOR_BLUE   := \033[0;34m
COLOR_YELLOW := \033[0;33m
COLOR_END    := \033[0m

# Tools
AR    = ar
CC    = gcc
CXX   = g++
RM    = rm -f
GDB   = gdb
MKDIR = mkdir -p
TOUCH = touch

# Handle OS X-specific tools.
ifeq ($(PLATFORM), Darwin)
	CC  = clang
	CXX = clang
	GDB = lldb
endif

# Flags
CFLAGS  = -Wall -Wno-psabi
LDFLAGS = -lm

# Enable Unicode on Windows platforms.
ifeq ($(PLATFORM), Windows)
	CFLAGS += -DUNICODE
endif

# Enable GNU Readline for Linux and OS X.
ifeq ($(PLATFORM), Linux)
	CFLAGS  += -DUSE_GNU_READLINE
	LDFLAGS += -lreadline
endif
ifeq ($(PLATFORM), Darwin)
	CFLAGS  += -DUSE_GNU_READLINE
	LDFLAGS += -lreadline
endif
