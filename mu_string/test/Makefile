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

CFLAGS := -Wall -O2 -I$(MULIB_DIR) -I$(TEST_DIR)  # Compiler flags
LDFLAGS :=                                        # Linker flags

.PHONY: run_unit_tests clean make_build_dirs

all: run_unit_tests

run_unit_tests: make_build_dirs $(UNIT_TESTS)
	@$(foreach exec, $(UNIT_TESTS), ./$(exec);)

# assure that the build directories exist
make_build_dirs:
	@mkdir -pv $(OBJ_DIR) $(BIN_DIR)

clean:
	rm -rvf $(OBJ_DIR) $(BIN_DIR)

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
