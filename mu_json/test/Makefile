# Compile and run unit tests
CC := gcc

MU_JSON_DIR := ../src
MU_STRING_DIR := ../../mu_string/src
TEST_DIR := .
OBJ_DIR := $(TEST_DIR)/obj
BIN_DIR := $(TEST_DIR)/bin

MU_JSON_SOURCES := \
	$(MU_JSON_DIR)/mu_jparse.c \
	$(MU_STRING_DIR)/mu_str.c

UNIT_TESTS := \
	$(BIN_DIR)/test_mu_parse

MU_JSON_OBJECTS := $(patsubst $(MU_JSON_DIR)/%.c,$(OBJ_DIR)/%.o,$(MU_JSON_SOURCES))
TEST_OBJECTS := $(patsubst $(BIN_DIR)/%,$(OBJ_DIR)/%.o,$(UNIT_TESTS))

 # Compiler flags
CFLAGS := -Wall -O0 -I$(MU_JSON_DIR) -I$(TEST_DIR)  \
	-I../../mu_string/src
COVERAGE_FLAGS := -fprofile-arcs -ftest-coverage

# Linker flags
LDFLAGS :=

.PHONY: unit_tests clean make_build_dirs coverage_tests

all: unit_tests

unit_tests: make_build_dirs $(UNIT_TESTS)
	@$(foreach exec, $(UNIT_TESTS), ./$(exec);)

# assure that the build directories exist
make_build_dirs:
	@mkdir -pv $(OBJ_DIR) $(BIN_DIR)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) coverage coverage.info

# generic rule for compiling MU_JSON_SOURCES
$(OBJ_DIR)/%.o: $(MU_JSON_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# generic rule for compiling TEST_SOURCES
$(OBJ_DIR)/%.o: $(TEST_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# the rule for buildiung each unit test executable is listed explicitly in order
# to guarantee separation of interests: test_mu_foo depends only on mu_foo.o

$(BIN_DIR)/test_mu_jparse: $(OBJ_DIR)/test_mu_jparse.o $(OBJ_DIR)/mu_jparse.o $(OBJ_DIR)/unity.o
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
