VASQ_DIR ?= .

VASQ_LIBNAME := vanillasquad
VASQ_SHARED_LIBRARY := $(VASQ_DIR)/lib$(VASQ_LIBNAME).so
VASQ_STATIC_LIBRARY := $(VASQ_DIR)/lib$(VASQ_LIBNAME).a

VASQ_OBJECT_FILES := $(patsubst %.c,%.o,$(wildcard $(VASQ_DIR)/source/*.c))
VASQ_INCLUDE_DIR := $(VASQ_DIR)/include
VASQ_HEADER_FILES := $(wildcard $(VASQ_DIR)/include/vasq/*.h)

VASQ_PHONY_TARGETS := vasq_clean

$(VASQ_SHARED_LIBRARY): $(VASQ_OBJECT_FILES)
	$(CC) -shared -o $@ $^

$(VASQ_STATIC_LIBRARY): $(VASQ_OBJECT_FILES)
	ar rcs $@ $^

$(VASQ_DIR)/source/%.o: $(VASQ_DIR)/source/%.c $(VASQ_HEADER_FILES)
	$(CC) $(COMPILER_FLAGS) -I$(VASQ_INCLUDE_DIR) -o $@ -c $<

vasq_clean:
	rm -f $(VASQ_SHARED_LIBRARY) $(VASQ_STATIC_LIBRARY) $(VASQ_OBJECT_FILES)
