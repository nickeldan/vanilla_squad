ifndef VASQ_MK
VASQ_MK :=

VASQ_LIB_DIR ?= $(VASQ_DIR)
VASQ_OBJ_DIR ?= $(VASQ_DIR)/source

VASQ_SHARED_LIBRARY := $(VASQ_LIB_DIR)/libvanillasquad.so
VASQ_STATIC_LIBRARY := $(VASQ_LIB_DIR)/libvanillasquad.a

VASQ_SOURCE_FILES := $(wildcard $(VASQ_DIR)/source/*.c)
VASQ_OBJECT_FILES := $(patsubst $(VASQ_DIR)/source/%.c,$(VASQ_OBJ_DIR)/%.o,$(VASQ_SOURCE_FILES))
VASQ_HEADER_FILES := $(wildcard $(VASQ_DIR)/include/vasq/*.h)
VASQ_INCLUDE_FLAGS := -I$(VASQ_DIR)/include

VASQ_DEPS_FILE := $(VASQ_OBJ_DIR)/deps.mk
DEPS_FILES += $(VASQ_DEPS_FILE)

BUILD_DEPS ?= $(if $(MAKECMDGOALS),$(subst clean,,$(MAKECMDGOALS)),yes)

ifneq ($(BUILD_DEPS),)

$(VASQ_DEPS_FILE): $(VASQ_SOURCE_FILES) $(VASQ_HEADER_FILES)
	@mkdir -p $(@D)
	@rm -f $@
	for file in $(VASQ_SOURCE_FILES); do \
	    echo "$(VASQ_OBJ_DIR)/`$(CC) $(VASQ_INCLUDE_FLAGS) -MM $$file`" >> $@ && \
	    echo '\t$$(CC) $$(CFLAGS) -fpic -ffunction-sections $(VASQ_INCLUDE_FLAGS) -c $$< -o $$@' >> $@; \
	done
include $(VASQ_DEPS_FILE)

endif

$(VASQ_SHARED_LIBRARY): $(VASQ_OBJECT_FILES)
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) -shared -o $@ $^

$(VASQ_STATIC_LIBRARY): $(VASQ_OBJECT_FILES)
	@mkdir -p $(@D)
	$(AR) rcs $@ $^

vasq_clean:
	@rm -f $(VASQ_SHARED_LIBRARY) $(VASQ_STATIC_LIBRARY) $(VASQ_OBJECT_FILES)

CLEAN_TARGETS += vasq_clean

endif
