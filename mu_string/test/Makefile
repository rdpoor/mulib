CC := gcc

LIB_DIR := ../src
TEST_DIR := .
OBJ_DIR := $(TEST_DIR)/obj
BIN_DIR := $(TEST_DIR)/bin

LIB_SOURCES := \
	$(LIB_DIR)/mu_str.c
LIB_HEADERS :=

UNIT_TESTS := \
	$(BIN_DIR)/test_mu_str

TEST_HEADERS := \
	fff.h \
	unity.h \
	unity_internals.h

LIB_OBJECTS := $(patsubst $(LIB_DIR)/%.c,$(OBJ_DIR)/%.o,$(LIB_SOURCES))
TEST_OBJECTS := $(patsubst $(BIN_DIR)/%,$(OBJ_DIR)/%.o,$(UNIT_TESTS))

CFLAGS := -Wall -O2 -I$(LIB_DIR) -I$(TEST_DIR)  # Compiler flags
LDFLAGS :=                                      # Linker flags

.PHONY: run_tests clean build_dirs_exist

all: run_tests

run_tests: make_build_dirs $(UNIT_TESTS)
	@$(foreach exec, $(UNIT_TESTS), ./$(exec);)

# assure that the build directories exist
make_build_dirs:
	@mkdir -pv $(OBJ_DIR) $(BIN_DIR)

clean:
	rm -rvf $(OBJ_DIR) $(BIN_DIR)

info:
	@echo LIB_SOURCES = $(LIB_SOURCES)
	@echo UNIT_TESTS = $(UNIT_TESTS)
	@echo LIB_OBJECTS = $(LIB_OBJECTS)
	@echo TEST_OBJECTS = $(TEST_OBJECTS)



# the dependencies for each unit test executable are listed explicitly in order
# to guarantee separation of interests: test_mu_foo depends only on mu_foo.o

$(BIN_DIR)/test_mu_str: $(OBJ_DIR)/test_mu_str.o $(OBJ_DIR)/mu_str.o $(OBJ_DIR)/unity.o
	$(CC) $(LDFLAGS) $^ -o $@

# generic rule for compiling LIB_SOURCES
$(OBJ_DIR)/%.o: $(LIB_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# generic rule for compiling TEST_SOURCES
$(OBJ_DIR)/%.o: $(TEST_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
