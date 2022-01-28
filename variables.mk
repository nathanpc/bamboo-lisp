### variables.mk
### Common variables used throught the project.
###
### Author: Nathan Campos <nathan@innoveworkshop.com>

# Environment
PLATFORM     := $(shell uname -s)
USE_PLOTTING := gnuplot

# Tools
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

# Enable plotting.
ifdef USE_PLOTTING
	CFLAGS += -DUSE_PLOTTING
endif

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
