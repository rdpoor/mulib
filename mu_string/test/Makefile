# Compile and run unit tests
CC := gcc

MULIB_DIR := ../src
TEST_DIR := .
OBJ_DIR := $(TEST_DIR)/obj
BIN_DIR := $(TEST_DIR)/bin

MULIB_SOURCES := \
	$(MULIB_DIR)/mu_str.c

UNIT_TESTS := \
	$(BIN_DIR)/test_mu_str

MULIB_OBJECTS := $(patsubst $(MULIB_DIR)/%.c,$(OBJ_DIR)/%.o,$(MULIB_SOURCES))
TEST_OBJECTS := $(patsubst $(BIN_DIR)/%,$(OBJ_DIR)/%.o,$(UNIT_TESTS))

CFLAGS := -Wall -O0 -I$(MULIB_DIR) -I$(TEST_DIR)  # Compiler flags
LDFLAGS :=                                        # Linker flags
COVERAGE_FLAGS := -fprofile-arcs -ftest-coverage

.PHONY: unit_tests clean make_build_dirs coverage_tests

all: unit_tests

unit_tests: make_build_dirs $(UNIT_TESTS)
	@$(foreach exec, $(UNIT_TESTS), ./$(exec);)

# assure that the build directories exist
make_build_dirs:
	@mkdir -pv $(OBJ_DIR) $(BIN_DIR)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) coverage coverage.info

# generic rule for compiling MULIB_SOURCES
$(OBJ_DIR)/%.o: $(MULIB_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# generic rule for compiling TEST_SOURCES
$(OBJ_DIR)/%.o: $(TEST_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# the rule for buildiung each unit test executable is listed explicitly in order
# to guarantee separation of interests: test_mu_foo depends only on mu_foo.o

$(BIN_DIR)/test_mu_str: $(OBJ_DIR)/test_mu_str.o $(OBJ_DIR)/mu_str.o $(OBJ_DIR)/unity.o
	$(CC) $(LDFLAGS) $^ -o $@

# coverage tests

coverage_tests: CFLAGS += $(COVERAGE_FLAGS)
coverage_tests: LDFLAGS += $(COVERAGE_FLAGS)
coverage_tests: clean make_build_dirs $(UNIT_TESTS)
	@$(foreach exec, $(UNIT_TESTS), ./$(exec);)
	lcov --capture --directory $(OBJ_DIR) --output-file coverage.info
	genhtml coverage.info --output-directory coverage
	@echo "Coverage report generated in 'coverage' directory."
	x-www-browser coverage/index.html
