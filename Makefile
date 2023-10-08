CFLAGS := -std=gnu99 -fdiagnostics-color -Wall -Wextra -Werror
ifeq ($(debug),yes)
    CFLAGS += -O0 -g -DDEBUG
else
    CFLAGS += -O2 -DNDEBUG
endif

all: _all

BUILD_DEPS :=
ifeq ($(MAKECMDGOALS),clean)
else ifeq ($(MAKECMDGOALS),format)
else
    BUILD_DEPS := yes
endif

VASQ_DIR := .
include make.mk

TEST_DIR := tests
include $(TEST_DIR)/make.mk

.PHONY: all _all format tests install uninstall clean $(CLEAN_TARGETS)

_all: $(VASQ_SHARED_LIBRARY) $(VASQ_STATIC_LIBRARY)

format:
	@find . -name '*.[hc]' -print0 | xargs -0 -n 1 clang-format -i

install: /usr/local/lib/$(notdir $(VASQ_SHARED_LIBRARY)) $(foreach file,$(VASQ_HEADER_FILES),/usr/local/include/vasq/$(notdir $(file)))
	ldconfig

/usr/local/lib/$(notdir $(VASQ_SHARED_LIBRARY)): $(VASQ_SHARED_LIBRARY)
	cp $< $@

/usr/local/include/vasq/%.h: include/vasq/%.h
	@mkdir -p $(@D)
	cp $< $@

uninstall:
	rm -rf /usr/local/include/vasq
	rm -f /usr/local/lib/$(notdir $(VASQ_SHARED_LIBRARY))
	ldconfig

clean: $(CLEAN_TARGETS)
	@rm -f $(DEPS_FILES)
