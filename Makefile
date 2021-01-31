CC ?= gcc
debug ?= no

COMPILER_FLAGS := -std=gnu11 -fpic -ffunction-sections -fdiagnostics-color -Wall -Wextra
ifeq ($(debug),yes)
    COMPILER_FLAGS += -O0 -g -DDEBUG
else
    COMPILER_FLAGS += -O3 -DNDEBUG
endif

TESTS := logger_test

.PHONY: all clean

include vasq.mk

all: $(VASQ_SHARED_LIBRARY) $(VASQ_STATIC_LIBRARY) $(TESTS)

%_test: tests/test_%.o $(VASQ_STATIC_LIBRARY)
	$(CC) -o $@ $^

tests/test_%.o: tests/test_%.c include/vasq/*.h
	cd tests && $(CC) $(COMPILER_FLAGS) -DVASQ_ENABLE_LOGGING -I../include -c $(notdir $<)

clean:
	rm -f $(VASQ_SHARED_LIBRARY) $(VASQ_STATIC_LIBRARY) $(VASQ_OBJECT_FILES) $(TESTS) tests/*.o
