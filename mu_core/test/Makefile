# Compile and run unit tests
CC := gcc
CFLAGS := -Wall -g
DEPFLAGS := -MMD -MP
GCOVFLAGS := -fprofile-arcs -ftest-coverage
# Add coverage flags also to the linker flags
LFLAGS := $(GCOVFLAGS)

SRC_DIR := ../src
TEST_DIR := ../test
TEST_SUPPORT_DIR := ../../test_support
OBJ_DIR := $(TEST_DIR)/obj
BIN_DIR := $(TEST_DIR)/bin
COVERAGE_DIR := $(TEST_DIR)/coverage

SRC_FILES := \
	$(SRC_DIR)/mu_array.c \
	$(SRC_DIR)/mu_queue.c \
	$(SRC_DIR)/mu_spsc.c

TEST_FILES := \
	$(TEST_DIR)/test_mu_array.c \
	$(TEST_DIR)/test_mu_queue.c \
	$(TEST_DIR)/test_mu_spsc.c

TEST_SUPPORT_FILES := \
	$(TEST_SUPPORT_DIR)/unity.c

SRC_OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))
TEST_OBJS := $(patsubst $(TEST_DIR)/%.c, $(OBJ_DIR)/%.o, $(TEST_FILES))
TEST_SUPPORT_OBJS := $(patsubst $(TEST_SUPPORT_DIR)/%.c, $(OBJ_DIR)/%.o, $(TEST_SUPPORT_FILES))
EXECUTABLES := $(patsubst $(TEST_DIR)/%.c, $(BIN_DIR)/%, $(TEST_FILES))

# Prevent makefile from automatically deleting object files
.SECONDARY: $(SRC_OBJS) $(TEST_OBJS) $(TEST_SUPPORT_OBJS)

# $(info SRC_OBJS = $(SRC_OBJS))
# $(info TEST_OBJS = $(TEST_OBJS))
# $(info TEST_SUPPORT_OBJS = $(TEST_SUPPORT_OBJS))
# $(info EXECUTABLES = $(EXECUTABLES))

.PHONY: all run-tests clean

all: $(EXECUTABLES)

run-tests: all
	@for test in $(EXECUTABLES) ; do \
		echo "Running $$test..."; \
		./$$test; \
	done

run-coverage:
	# Determine absolute paths
	# TEST_DIR_ABS=$$(realpath $(TEST_DIR))
	# TEST_SUPPORT_DIR_ABS=$$(realpath $(TEST_SUPPORT_DIR))

	# Clean and rebuild everything with coverage flags
	$(MAKE) clean
	$(MAKE) all CFLAGS="$(CFLAGS) $(GCOVFLAGS)"
	# Run tests to generate coverage data
	@for test in $(EXECUTABLES) ; do \
		./$$test; \
	done
	# Capture initial coverage data
	lcov --capture --directory $(OBJ_DIR) --output-file coverage.info
	# Remove coverage data for test and test_support directories using absolute paths
	# lcov --remove coverage.info "$(TEST_DIR_ABS)/*" "$(TEST_SUPPORT_DIR_ABS)/*" --output-file coverage.info.cleaned
	# Generate coverage report
	genhtml coverage.info --output-directory $(COVERAGE_DIR)
	@echo "Coverage report generated in $(COVERAGE_DIR)"
	# Optional: Clean up intermediate coverage files
	# rm -f coverage.info coverage.info.cleaned

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(COVERAGE_DIR)

# Compile and generate dependencies for source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -I$(SRC_DIR) $(DEPFLAGS) -c $< -o $@

# Compile and generate dependencies for test files
$(OBJ_DIR)/%.o: $(TEST_DIR)/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -I$(TEST_SUPPORT_DIR) $(DEPFLAGS) -c $< -o $@

# Compile and generate dependencies for test support files
$(OBJ_DIR)/%.o: $(TEST_SUPPORT_DIR)/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -I$(SRC_DIR) $(DEPFLAGS) -c $< -o $@

-include $(OBJ_DIR)/*.d

# Link object files to create executables
$(BIN_DIR)/%: $(OBJ_DIR)/%.o $(SRC_OBJS) $(TEST_SUPPORT_OBJS)
	mkdir -p $(BIN_DIR)
	$(CC) $(LFLAGS) $^ -o $@
