### Makefile
### Automates the build of the Bamboo examples.
###
### Author: Nathan Campos <nathan@innoveworkshop.com>

include ../variables.mk

# Sources and Flags
BAMBOODIR := ../src
TARGETS = improved_repl basic_repl
CXXTARGETS = cpp_repl
CFLAGS = -Wall -Wno-psabi
LDFLAGS = 

.PHONY: all clean
all: $(TARGETS) $(CXXTARGETS)

$(TARGETS): bamboo.o

$(CXXTARGETS): bamboo.o BambooWrapper.o

bamboo.o: $(BAMBOODIR)/bamboo.c
	$(CC) $(CFLAGS) -c $< -o $@

BambooWrapper.o: $(BAMBOODIR)/BambooWrapper.cpp
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
