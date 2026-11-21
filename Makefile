.DEFAULT_GOAL := all

CC ?= cc
PYTHON ?= python3
JAVA ?= java
JAVAC ?= javac
CFLAGS ?= -O2 -g
WARNINGS := -std=c17 -Wall -Wextra -Wpedantic -Werror

SOURCES := $(sort $(wildcard examples/*/*.c))
PROGRAMS := $(patsubst examples/%.c,build/%,$(SOURCES))
JAVA_SOURCES := $(sort $(wildcard examples/*/*.java))

.PHONY: all java test test-python test-java test-all clean

all: $(PROGRAMS)

build/threads/%: THREAD_FLAGS := -pthread

build/%: examples/%.c Makefile
	@mkdir -p "$(@D)"
	$(CC) $(CPPFLAGS) $(CFLAGS) $(WARNINGS) $(THREAD_FLAGS) $(LDFLAGS) "$<" $(LDLIBS) -o "$@"

java:
	@mkdir -p build/java
	$(JAVAC) --release 17 -encoding UTF-8 -Xlint:all -Werror -d build/java $(JAVA_SOURCES)

test: all
	sh tests/smoke.sh c

test-python:
	PYTHON="$(PYTHON)" sh tests/smoke.sh python

test-java: java
	JAVA="$(JAVA)" sh tests/smoke.sh java

test-all: test test-python test-java

clean:
	rm -rf -- build
