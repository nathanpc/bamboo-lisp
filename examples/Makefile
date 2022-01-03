### Makefile
### Automates the build of the Bamboo examples.
###
### Author: Nathan Campos <nathan@innoveworkshop.com>

# Tools
CC = gcc
CXX = g++
RM = rm -f
GDB = gdb
MKDIR = mkdir -p
TOUCH = touch

# Sources and Flags
TARGETS = repl
CXXTARGETS = cpp_repl
CFLAGS = -Wall
LDFLAGS = 

.PHONY: all clean
all: $(TARGETS) $(CXXTARGETS)

$(TARGETS): bamboo.o

$(CXXTARGETS): bamboo.o

bamboo.o: ../src/bamboo.c
	$(CC) $(CFLAGS) -c $< -o $@

BambooWrapper.o: ../src/BambooWrapper.cpp
	$(CXX) $(CFLAGS) $(CXXFLAGS) -c $< -o $@

$(TARGETS): %: %.o bamboo.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(CXXTARGETS): %: %.o bamboo.o BambooWrapper.o
	$(CXX) $(CFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) *.o
	$(RM) $(TARGETS)
	$(RM) $(CXXTARGETS)
