TESTS := $(patsubst $(TEST_DIR)/test_%.c,$(TEST_DIR)/%,$(wildcard $(TEST_DIR)/test_*.c))

$(TEST_DIR)/%: $(TEST_DIR)/test_%.c $(VASQ_STATIC_LIBRARY)
	$(CC) $(CFLAGS) $(VASQ_INCLUDE_FLAGS) -o $@ $^

tests: $(TESTS) $(wildcard $(TEST_DIR)/*.py)
	pytest $(TEST_DIR)

test_clean:
	@rm -f $(TESTS)

CLEAN_TARGETS += test_clean
