CC ?= gcc
debug ?= no

CFLAGS := -std=gnu11 -fdiagnostics-color -Wall -Wextra
ifeq ($(debug),yes)
    CFLAGS += -O0 -g -DDEBUG
else
    CFLAGS += -O2 -DNDEBUG
endif

TESTS := logger_test

all: _all

VASQ_DIR := .
include make.mk

.PHONY: all _all clean $(CLEAN_TARGETS)

_all: $(VASQ_SHARED_LIBRARY) $(VASQ_STATIC_LIBRARY) $(TESTS)

%_test: tests/test_%.o $(VASQ_STATIC_LIBRARY)
	$(CC) -o $@ $^

tests/test_%.o: tests/test_%.c $(VASQ_HEADER_FILES)
	$(CC) $(CFLAGS) -I$(VASQ_INCLUDE_DIR) -c $< -o $@

clean: $(CLEAN_TARGETS)
	rm -f $(TESTS) $(DEPS_FILES)
