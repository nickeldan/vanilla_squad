VASQ_LIBNAME := vanillasquad
VASQ_SHARED_LIBRARY := $(VASQ_DIR)/lib$(VASQ_LIBNAME).so
VASQ_STATIC_LIBRARY := $(VASQ_DIR)/lib$(VASQ_LIBNAME).a

VASQ_SOURCE_FILES := $(wildcard $(VASQ_DIR)/source/*.c)
VASQ_OBJECT_FILES := $(patsubst %.c,%.o,$(VASQ_SOURCE_FILES))
VASQ_HEADER_FILES := $(wildcard $(VASQ_DIR)/include/vasq/*.h)
VASQ_INCLUDE_DIR := $(VASQ_DIR)/include

VASQ_DEPS_FILE := $(VASQ_DIR)/deps.mk
DEPS_FILES += $(VASQ_DEPS_FILE)

CLEAN_TARGETS += vasq_clean

$(VASQ_DEPS_FILE): $(VASQ_SOURCE_FILES) $(VASQ_HEADER_FILES)
	rm -f $@
	for file in $(VASQ_SOURCE_FILES); do \
	    echo "$(VASQ_DIR)/source/`$(CC) -I$(VASQ_INCLUDE_DIR) -MM $$file`" >> $@ && \
	    echo '\t$$(CC) $$(CFLAGS) -fpic -ffunction-sections -I$(VASQ_INCLUDE_DIR) -c $$< -o $$@' >> $@; \
	done
include $(VASQ_DEPS_FILE)

$(VASQ_SHARED_LIBRARY): $(VASQ_OBJECT_FILES)
	$(CC) -shared -o $@ $^

$(VASQ_STATIC_LIBRARY): $(VASQ_OBJECT_FILES)
	$(AR) rcs $@ $^

vasq_clean:
	@rm -f $(VASQ_SHARED_LIBRARY) $(VASQ_STATIC_LIBRARY) $(VASQ_OBJECT_FILES)
