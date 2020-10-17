CC ?= gcc
debug ?= no

COMPILER_FLAGS := -std=gnu11 -fdiagnostics-color -Wall -Wextra
ifeq ($(debug),yes)
    COMPILER_FLAGS += -O0 -g -DDEBUG
else
    COMPILER_FLAGS += -O3 -DNDEBUG
endif

LIBNAME := libvanillasquad
OBJECT_FILES := source/logger.o source/safe_snprintf.o source/unittest.o

.PHONY: all clean

all: $(LIBNAME).so $(LIBNAME).a

$(LIBNAME).so: $(OBJECT_FILES)
	$(CC) -shared -o $@ $^

$(LIBNAME).a: $(OBJECT_FILES)
	ar rcs $@ $^

source/%.o: source/%.c include/vasq/*.h
	cd source && $(CC) $(COMPILER_FLAGS) -I../include -c $(notdir $<)

clean:
	rm -f $(LIBNAME).so $(LIBNAME).a source/*.o
