PROJECT = bamboo
PLATFORM := $(shell uname -s)

CC = gcc
RM = rm -f
GDB = gdb
MKDIR = mkdir -p

SRCDIR = src
BUILDDIR := build
TARGET = $(BUILDDIR)/$(PROJECT)

SOURCES += $(SRCDIR)/main.c $(SRCDIR)/bamboo.c
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.c=.o))
CFLAGS = -Wall
LDFLAGS = 

.PHONY: all run test debug memcheck clean
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	$(TARGET)

debug: CFLAGS += -g3 # -DDEBUG
debug: clean $(TARGET)
	$(GDB) $(TARGET)

memcheck: CFLAGS += -g3 -DDEBUG -DMEMCHECK
memcheck: clean $(TARGET)
	valgrind --tool=memcheck --leak-check=yes --show-leak-kinds=all --track-origins=yes --log-file=valgrind.log $(TARGET)
	cat valgrind.log

clean:
	$(RM) -r $(BUILDDIR)
	$(RM) valgrind.log
