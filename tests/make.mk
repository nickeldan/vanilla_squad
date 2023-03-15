TEST_BINARY := $(TEST_DIR)/tests
TEST_SOURCE_FILES := $(wildcard $(TEST_DIR)/*.c)
TEST_OBJECT_FILES := $(patsubst %.c,%.o,$(TEST_SOURCE_FILES))

TEST_DEPS_FILE := $(TEST_DIR)/deps.mk
DEPS_FILES += $(TEST_DEPS_FILE)

BUILD_DEPS ?= $(if $(MAKECMDGOALS),$(subst clean,,$(MAKECMDGOALS)),yes)

ifneq ($(BUILD_DEPS),)

$(TEST_DEPS_FILE): $(TEST_SOURCE_FILES) $(VASQ_HEADER_FILES)
	@mkdir -p $(@D)
	@rm -f $@
	for file in $(TEST_SOURCE_FILES); do \
	    echo "$(TEST_DIR)/`$(CC) $(VASQ_INCLUDE_FLAGS) -MM $$file`" >> $@ && \
	    echo '\t$$(CC) $$(CFLAGS) -DVASQ_TEST_ASSERT -DDEBUG $(VASQ_INCLUDE_FLAGS) -c $$< -o $$@' >> $@; \
	done
include $(TEST_DEPS_FILE)

endif


$(TEST_BINARY): $(TEST_OBJECT_FILES) $(VASQ_SHARED_LIBRARY)
	$(CC) $(CFLAGS) $(VASQ_INCLUDE_FLAGS) $(filter %.o,$^) -Wl,-rpath $(realpath $(VASQ_LIB_DIR)) -L$(VASQ_LIB_DIR) -lvanillasquad -lscrutiny -o $@

tests: $(TEST_BINARY)
	@$<

tests_clean:
	@rm -f $(TEST_BINARY) $(TEST_OBJECT_FILES)

CLEAN_TARGETS += tests_clean
