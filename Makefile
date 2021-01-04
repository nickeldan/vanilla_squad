CC ?= gcc
debug ?= no

LIBNAME := vanillasquad
COMPILER_FLAGS := -std=gnu11 -fpic -fdiagnostics-color -Wall -Wextra
ifeq ($(debug),yes)
    COMPILER_FLAGS += -O0 -g -DDEBUG
else
    COMPILER_FLAGS += -O3 -DNDEBUG
endif

SHARED_LIBRARY := lib$(LIBNAME).so
STATIC_LIBRARY := lib$(LIBNAME).a
OBJECT_FILES := $(patsubst %.c,%.o,$(wildcard source/*.c))
TESTS := logger_test

.PHONY: all clean

all: $(SHARED_LIBRARY) $(STATIC_LIBRARY) $(TESTS)

$(SHARED_LIBRARY): $(OBJECT_FILES)
	$(CC) -shared -o $@ $^

$(STATIC_LIBRARY): $(OBJECT_FILES)
	ar rcs $@ $^

source/%.o: source/%.c include/vasq/*.h
	cd source && $(CC) $(COMPILER_FLAGS) -ffunction-sections -I../include -c $(notdir $<)

%_test: tests/test_%.o lib$(LIBNAME).a
	$(CC) -o $@ $^

tests/test_%.o: tests/test_%.c include/vasq/*.h
	cd tests && $(CC) $(COMPILER_FLAGS) -DVASQ_ENABLE_LOGGING -I../include -c $(notdir $<)

clean:
	rm -f $(SHARED_LIBRARY) $(STATIC_LIBRARY) $(OBJECT_FILES) $(TESTS) tests/*.o
