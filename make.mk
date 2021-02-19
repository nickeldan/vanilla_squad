VASQ_LIBNAME := vanillasquad
VASQ_SHARED_LIBRARY := $(VASQ_DIR)/lib$(VASQ_LIBNAME).so
VASQ_STATIC_LIBRARY := $(VASQ_DIR)/lib$(VASQ_LIBNAME).a

VASQ_SOURCE_FILES := $(wildcard $(VASQ_DIR)/source/*.c)
VASQ_OBJECT_FILES := $(patsubst %.c,%.o,$(VASQ_SOURCE_FILES))
VASQ_HEADER_FILES := $(wildcard $(VASQ_DIR)/include/vasq/*.h)
VASQ_INCLUDE_DIR := $(VASQ_DIR)/include

CLEAN_TARGETS += vasq_clean

$(VASQ_DIR)/deps.mk: $(VASQ_SOURCE_FILES) $(VASQ_HEADER_FILES)
	rm -f $@
	for file in $(VASQ_SOURCE_FILES); do \
	    echo "$(VASQ_DIR)/source/`$(CC) -MM $$file`" >> $@ && \
	    echo '\t$$(CC) $$(CFLAGS) -fpic -ffunction-sections -c $$< -o $$@' >> $@; \
	done
include $(VASQ_DIR)/deps.mk

$(VASQ_SHARED_LIBRARY): $(VASQ_OBJECT_FILES)
	$(CC) -shared -o $@ $^

$(VASQ_STATIC_LIBRARY): $(VASQ_OBJECT_FILES)
	ar rcs $@ $^

vasq_clean:
	rm -f $(VASQ_SHARED_LIBRARY) $(VASQ_STATIC_LIBRARY) $(VASQ_OBJECT_FILES) $(VASQ_DIR)/deps.mk
