CC ?= gcc
debug ?= no

CFLAGS := -std=gnu99 -fdiagnostics-color -Wall -Wextra
ifeq ($(debug),yes)
    CFLAGS += -O0 -g -DDEBUG
else
    CFLAGS += -O2 -DNDEBUG
endif

TESTS := logger_test snprintf_test assert_test

all: _all

VASQ_DIR := .
include make.mk

.PHONY: all _all tests format install uninstall clean $(CLEAN_TARGETS)

_all: $(VASQ_SHARED_LIBRARY) $(VASQ_STATIC_LIBRARY)

tests: $(TESTS)

format:
	find . -name '*.[hc]' -print0 | xargs -0 -n 1 clang-format -i

install: /usr/local/lib/$(notdir $(VASQ_SHARED_LIBRARY)) $(foreach file,$(VASQ_HEADER_FILES),/usr/local/include/vasq/$(notdir $(file)))

/usr/local/lib/$(notdir $(VASQ_SHARED_LIBRARY)): $(VASQ_SHARED_LIBRARY)
	cp $< $@

/usr/local/include/vasq/%.h: $(VASQ_INCLUDE_DIR)/vasq/%.h /usr/local/include/vasq
	cp $< $@

/usr/local/include/vasq:
	mkdir -p $@

uninstall:
	rm -rf /usr/local/include/vasq
	rm -f /usr/local/lib/$(notdir $(VASQ_SHARED_LIBRARY))

%_test: tests/test_%.o $(VASQ_STATIC_LIBRARY)
	$(CC) -o $@ $^

tests/test_%.o: tests/test_%.c $(VASQ_HEADER_FILES)
	$(CC) $(CFLAGS) -I$(VASQ_INCLUDE_DIR) -c $< -o $@

clean: $(CLEAN_TARGETS)
	@rm -f $(TESTS) $(DEPS_FILES)
