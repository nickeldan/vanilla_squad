CC ?= gcc
debug ?= no

COMPILER_FLAGS := -std=gnu11 -fpic -ffunction-sections -fdiagnostics-color -Wall -Wextra
ifeq ($(debug),yes)
    COMPILER_FLAGS += -O0 -g -DDEBUG
else
    COMPILER_FLAGS += -O3 -DNDEBUG
endif

TESTS := logger_test

include vasq.mk

.PHONY: all clean $(VASQ_PHONY_TARGETS)

all: $(VASQ_SHARED_LIBRARY) $(VASQ_STATIC_LIBRARY) $(TESTS)

%_test: tests/test_%.o $(VASQ_STATIC_LIBRARY)
	$(CC) -o $@ $^

tests/test_%.o: tests/test_%.c include/vasq/*.h
	cd tests && $(CC) $(COMPILER_FLAGS) -DVASQ_ENABLE_LOGGING -I../include -c $(notdir $<)

clean: vasq_clean
