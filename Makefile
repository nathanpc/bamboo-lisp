### Makefile
### Automates the build of the project.
###
### Author: Nathan Campos <nathan@innoveworkshop.com>

include variables.mk

# Directories and Paths
LISPDIR  := lisp
REPLDIR  := tinyrepl
BUILDDIR := build

# Targets
REPLEXE   := $(BUILDDIR)/$(REPLDIR)/$(REPLDIR)
BAMBOOLIB := $(BUILDDIR)/$(LISPDIR)/libbamboo.a

.PHONY: all compile run test debug memcheck clean
all: compile

compile: $(BUILDDIR)/stamp
	cd $(LISPDIR) && $(MAKE) compile
	cd $(REPLDIR) && $(MAKE) compile

$(BUILDDIR)/stamp:
	$(MKDIR) $(@D)
	$(TOUCH) $@

run: compile
	cd $(REPLDIR) && $(MAKE) run

debug: $(BUILDDIR)/stamp
	cd $(LISPDIR) && $(MAKE) debug
	cd $(REPLDIR) && $(MAKE) debug

memcheck: $(BUILDDIR)/stamp clean
	cd $(LISPDIR) && $(MAKE) memcheck
	cd $(REPLDIR) && $(MAKE) memcheck

clean:
	cd $(LISPDIR) && $(MAKE) clean
	cd $(REPLDIR) && $(MAKE) clean
