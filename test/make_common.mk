
TEST_DIR := .
TEST_SUPPORT_DIR := ../test_support
TEST_FILE := $(TEST_DIR)/$(MODULE_NAME).c
OBJ_DIR := $(TEST_DIR)/obj
BIN_DIR := $(TEST_DIR)/bin
COVERAGE_DIR := $(TEST_DIR)/coverage

CC := gcc
CFLAGS := -Wall -g $(OTHER_FLAGS)
DEPFLAGS := -MMD -MP
GCOVFLAGS := -fprofile-arcs -ftest-coverage
INCLUDES := -I$(SRC_DIR) -I$(TEST_SUPPORT_DIR) -I..
# Add coverage flags also to the linker flags
LFLAGS := $(GCOVFLAGS)

TEST_SUPPORT_FILES := \
	$(TEST_SUPPORT_DIR)/unity.c

SRC_OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))
TEST_OBJS := $(patsubst $(TEST_DIR)/%.c, $(OBJ_DIR)/%.o, $(TEST_FILE))
TEST_SUPPORT_OBJS := $(patsubst $(TEST_SUPPORT_DIR)/%.c, $(OBJ_DIR)/%.o, $(TEST_SUPPORT_FILES))
EXECUTABLE := $(BIN_DIR)/$(MODULE_NAME)

.PHONY: all tests coverage clean

# Prevent makefile from automatically deleting object files
.SECONDARY: $(SRC_OBJS) $(TEST_OBJS) $(TEST_SUPPORT_OBJS)

# $(info SRC_OBJS = $(SRC_OBJS))
# $(info TEST_OBJS = $(TEST_OBJS))
# $(info TEST_SUPPORT_OBJS = $(TEST_SUPPORT_OBJS))
# $(info EXECUTABLES = $(EXECUTABLES))


all: $(EXECUTABLE)

tests: all
	echo "Running $(EXECUTABLE)..."
	$(EXECUTABLE)

coverage:
	# Determine absolute paths
	# TEST_DIR_ABS=$$(realpath $(TEST_DIR))
	# TEST_SUPPORT_DIR_ABS=$$(realpath $(TEST_SUPPORT_DIR))

	# Clean and rebuild everything with coverage flags
	$(MAKE) clean
	$(MAKE) all CFLAGS="$(CFLAGS) $(GCOVFLAGS) $(OTHER_FLAGS)"
	# Run tests to generate coverage data
	$(EXECUTABLE)
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
	$(CC) $(CFLAGS) $(INCLUDES) $(DEPFLAGS) $(OTHER_FLAGS) -c $< -o $@

# Compile and generate dependencies for test files
$(OBJ_DIR)/%.o: $(TEST_DIR)/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) $(OTHER_FLAGS) $(DEPFLAGS) -c $< -o $@

# Compile and generate dependencies for test support files
$(OBJ_DIR)/%.o: $(TEST_SUPPORT_DIR)/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) $(DEPFLAGS) -c $< -o $@

-include $(OBJ_DIR)/*.d

# Link object files to create executables
$(BIN_DIR)/%: $(OBJ_DIR)/%.o $(SRC_OBJS) $(TEST_SUPPORT_OBJS)
	mkdir -p $(BIN_DIR)
	$(CC) $(LFLAGS) $^ -o $@
