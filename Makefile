### Makefile
### Building instructions for Bamboo Lisp using Open Watcom's Make utility.
###
### Author: Nathan Campos <nathan@innoveworkshop.com>

# Settings
PROJECT = bamboo
PLATFORM = nt

# Paths
SRCDIR = src
BUILDDIR = build
OBJS = $(BUILDDIR)/main.obj $(BUILDDIR)/bamboo.obj
BIN  = $(BUILDDIR)/$(PROJECT).exe

# Programs
CC = wcc386
LD = wlink
RM = del /F /Q
RMDIR = rmdir /Q

# Flags
CFLAGS = -w4 -e25 -zq -od -d2 -6r -bt=$(PLATFORM) -mf
LDFLAGS = d all sys $(PLATFORM) op m op maxe=25 op q op symf

all: $(BIN) .symbolic

$(BUILDDIR)/bamboo.obj: $(SRCDIR)/bamboo.c
	$(CC) $< $(CFLAGS) -fo=$@

$(BUILDDIR)/main.obj: $(SRCDIR)/main.c
	$(CC) $< $(CFLAGS) -fo=$@

$(BIN): $(BUILDDIR)/ $(OBJS)
	$(LD) name $@ file { $(OBJS) }

$(BUILDDIR)/:
	mkdir $(BUILDDIR)

run: $(BIN) .symbolic
	$<

clean: .symbolic
	$(RM) $(BUILDDIR)\*
	$(RMDIR) $(BUILDDIR)
