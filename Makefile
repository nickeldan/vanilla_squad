CC ?= gcc
debug ?= no

COMPILER_FLAGS := -std=gnu11 -fpic -ffunction-sections -fdiagnostics-color -Wall -Wextra
ifeq ($(debug),yes)
    COMPILER_FLAGS += -O0 -g -DDEBUG
else
    COMPILER_FLAGS += -O3 -DNDEBUG
endif

TESTS := logger_test

_all: all

include vasq.mk

.PHONY: _all all clean $(VASQ_PHONY_TARGETS)

all: $(VASQ_SHARED_LIBRARY) $(VASQ_STATIC_LIBRARY) $(TESTS)

%_test: tests/test_%.o $(VASQ_STATIC_LIBRARY)
	$(CC) -o $@ $^

tests/test_%.o: tests/test_%.c $(VASQ_HEADER_FILES)
	$(CC) $(COMPILER_FLAGS) -DVASQ_ENABLE_LOGGING -I$(VASQ_INCLUDE_DIR) -c $< -o $@

clean: vasq_clean
