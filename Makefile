.DEFAULT_GOAL := all

CC ?= cc
CFLAGS ?= -O2 -g
WARNINGS := -std=c17 -Wall -Wextra -Wpedantic -Werror

SOURCES := $(sort $(wildcard examples/*/*.c))
PROGRAMS := $(patsubst examples/%.c,build/%,$(SOURCES))

.PHONY: all clean

all: $(PROGRAMS)

build/threads/%: THREAD_FLAGS := -pthread

build/%: examples/%.c Makefile
	@mkdir -p "$(@D)"
	$(CC) $(CPPFLAGS) $(CFLAGS) $(WARNINGS) $(THREAD_FLAGS) $(LDFLAGS) "$<" $(LDLIBS) -o "$@"

clean:
	rm -rf -- build
