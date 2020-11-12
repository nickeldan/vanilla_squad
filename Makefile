CC ?= gcc
debug ?= no

COMPILER_FLAGS := -std=gnu11 -fpic -fdiagnostics-color -Wall -Wextra
ifeq ($(debug),yes)
    COMPILER_FLAGS += -O0 -g -DDEBUG
else
    COMPILER_FLAGS += -O3 -DNDEBUG
endif

LIBNAME := vanillasquad
OBJECT_FILES := source/logger.o source/safe_snprintf.o source/unittest.o
TESTS := logger_test

.PHONY: all clean

all: lib$(LIBNAME).so lib$(LIBNAME).a $(TESTS)

lib$(LIBNAME).so: $(OBJECT_FILES)
	$(CC) -shared -o $@ $^

lib$(LIBNAME).a: $(OBJECT_FILES)
	ar rcs $@ $^

source/%.o: source/%.c include/vasq/*.h
	cd source && $(CC) $(COMPILER_FLAGS) -I../include -c $(notdir $<)

%_test: tests/test_%.o lib$(LIBNAME).a
	$(CC) -o $@ $^

tests/test_%.o: tests/test_%.c include/vasq/*.h
	cd tests && $(CC) $(COMPILER_FLAGS) -DVASQ_ENABLE_LOGGING -I../include -c $(notdir $<)

clean:
	rm -f lib$(LIBNAME).so lib$(LIBNAME).a $(TESTS) source/*.o tests/*.o
