# make-common.mk
# Compile and run unit tests and coverage tests

SRC_DIR := ../src
TEST_DIR := .
TEST_SUPPORT_DIR := ./test_support
OBJ_DIR := $(TEST_DIR)/obj
BIN_DIR := $(TEST_DIR)/bin
COVERAGE_DIR := $(TEST_DIR)/coverage

# $(CONFIG_FILE) must define SRC_FILES, TEST_FILES and optionally OTHER_FLAGS
include $(CONFIG_FILE)

CC := gcc
CFLAGS := -Wall -g $(OTHER_FLAGS)
DEPFLAGS := -MMD -MP
GCOVFLAGS := -fprofile-arcs -ftest-coverage
# Add coverage flags also to the linker flags
LFLAGS := $(GCOVFLAGS)

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

.PHONY: all tests coverage clean

all: $(EXECUTABLES)

tests: all
	@for test in $(EXECUTABLES) ; do \
		echo "Running $$test..."; \
		./$$test; \
	done

coverage:
	# Determine absolute paths
	# TEST_DIR_ABS=$$(realpath $(TEST_DIR))
	# TEST_SUPPORT_DIR_ABS=$$(realpath $(TEST_SUPPORT_DIR))

	# Clean and rebuild everything with coverage flags
	$(info OTHER_FLAGS = $(OTHER_FLAGS))
	$(MAKE) clean
	$(MAKE) all CFLAGS="$(CFLAGS) $(GCOVFLAGS) $(OTHER_FLAGS)"
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
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(COVERAGE_DIR) coverage.info

# Compile and generate dependencies for source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -I$(SRC_DIR) $(DEPFLAGS) $(OTHER_FLAGS) -c $< -o $@

# Compile and generate dependencies for test files
# -I. is a hack since 
$(OBJ_DIR)/%.o: $(TEST_DIR)/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -I$(TEST_SUPPORT_DIR) $(OTHER_FLAGS) $(DEPFLAGS) -c $< -o $@

# Compile and generate dependencies for test support files
$(OBJ_DIR)/%.o: $(TEST_SUPPORT_DIR)/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -I$(SRC_DIR) $(DEPFLAGS) -c $< -o $@

-include $(OBJ_DIR)/*.d

# Link object files to create executables
$(BIN_DIR)/%: $(OBJ_DIR)/%.o $(SRC_OBJS) $(TEST_SUPPORT_OBJS)
	mkdir -p $(BIN_DIR)
	$(CC) $(LFLAGS) $^ -o $@
