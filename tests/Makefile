# As part of the unit testing process, this makefile compiles all of the sources
# in tests/, writing the .o object files into build/, then creates and runs the
# executable build/mu_tests

.PHONY: all clean

BUILD_DIR = ./build
CORE_SOURCE_DIR = ../mulib/core
CORE_TEST_DIR = ./core
TEST_SUPPORT_DIR = ./test_support

CORE_SOURCE_SOURCES := $(wildcard $(CORE_SOURCE_DIR)/*.c)
CORE_SOURCE_INCLUDES := $(wildcard $(CORE_SOURCE_DIR)/*.h)
CORE_TEST_SOURCES := $(wildcard $(CORE_TEST_DIR)/*.c)
CORE_TEST_INCLUDES := $(wildcard $(CORE_TEST_DIR)/*.h)

ALL_OBJECTS = $(wildcard $(BUILD_DIR)/*.o)

$(shell echo $(ALL_OBJECTS))

UNIT_TEST := $(BUILD_DIR)/mu_test

CFLAGS = -Wall -g

IFLAGS = -I $(CORE_SOURCE_DIR) -I $(TEST_SUPPORT_DIR)

# DIRS = ../build ../build/platform
$(shell mkdir -p $(BUILD_DIR))

# build ../src/*.o
$(BUILD_DIR)/%.o : $(TEST_DRIVER_DIR)/%.c $(TEST_DRIVER_INCLUDES)
	$(CC) $(CFLAGS) $(IFLAGS) -c $(<) -o $@

# link all .o files into mu_test executable
$(UNIT_TEST) : $(ALL_OBJECTS)

all : $(ALL_OBJECTS)
	ls $(ALL_OBJECTS)

# run the unit tests
test : all
	$(UNIT_TEST)

clean :
	rm -rf $(BUILD_DIR)

