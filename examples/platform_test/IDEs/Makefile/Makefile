# Makefile for the oblique_eg example
#
# `make` will compile the example.
# `make run` will build and run the example.
# `make clean` will clean all

.PHONY: all clean mulib_core_objects mulib_extras_objects mu_platform_objects oblique_eg_objects

APP_NAME = platform_test

TOP_MULIB_DIR = ../../../..
MODULES_DIR = $(TOP_MULIB_DIR)/mulib

#i would love to get this to work
#MODULES_USED = collections log sched string term utils

SRC_MULIB_COLLECTIONS_DIR = $(MODULES_DIR)/collections/src
SRC_MULIB_LOG_DIR = $(MODULES_DIR)/log/src
SRC_MULIB_SCHED_DIR = $(MODULES_DIR)/sched/src
SRC_MULIB_STRING_DIR = $(MODULES_DIR)/string/src
SRC_MULIB_TERM_DIR = $(MODULES_DIR)/term/src
SRC_MULIB_UTILS_DIR = $(MODULES_DIR)/utils/src

INC_MULIB_COLLECTIONS_DIR = $(MODULES_DIR)/collections/inc
INC_MULIB_LOG_DIR = $(MODULES_DIR)/log/inc
INC_MULIB_SCHED_DIR = $(MODULES_DIR)/sched/inc
INC_MULIB_STRING_DIR = $(MODULES_DIR)/string/inc
INC_MULIB_TERM_DIR = $(MODULES_DIR)/term/inc
INC_MULIB_UTILS_DIR = $(MODULES_DIR)/utils/inc

INC_MU_PLATFORM_DIR = $(TOP_MULIB_DIR)/platforms/inc
SRC_MU_PLATFORM_DIR = $(TOP_MULIB_DIR)/platforms/src/Makefile/POSIX

INC_APP_DIR = $(TOP_MULIB_DIR)/examples/$(APP_NAME)/inc
SRC_APP_DIR = $(TOP_MULIB_DIR)/examples/$(APP_NAME)/src

SOURCES_MU_COLLECTIONS := $(wildcard $(SRC_MULIB_COLLECTIONS_DIR)/*.c)
SOURCES_MU_LOG := $(wildcard $(SRC_MULIB_LOG_DIR)/*.c)
SOURCES_MU_SCHED := $(wildcard $(SRC_MULIB_SCHED_DIR)/*.c)
SOURCES_MU_STRING := $(wildcard $(SRC_MULIB_STRING_DIR)/*.c)
SOURCES_MU_TERM := $(wildcard $(SRC_MULIB_TERM_DIR)/*.c)
SOURCES_MU_UTILS := $(wildcard $(SRC_MULIB_UTILS_DIR)/*.c)

SOURCES_MU_PLATFORM := $(wildcard $(SRC_MU_PLATFORM_DIR)/*.c)
SOURCES_APP := $(wildcard $(SRC_APP_DIR)/*.c)

BUILD_DIR := build
BUILD_MULIB_DIR := $(BUILD_DIR)/mulib
BUILD_MU_PLATFORM_DIR := $(BUILD_DIR)/mu_platform
BUILD_APP_DIR := $(BUILD_DIR)/$(APP_NAME)

OBJECTS_MU_COLLECTIONS ?= $(patsubst $(SRC_MULIB_COLLECTIONS_DIR)/%.c, $(BUILD_MULIB_DIR)/collections/%.o, $(SOURCES_MU_COLLECTIONS))
OBJECTS_MU_LOG ?= $(patsubst $(SRC_MULIB_LOG_DIR)/%.c, $(BUILD_MULIB_DIR)/log/%.o, $(SOURCES_MU_LOG))
OBJECTS_MU_SCHED ?= $(patsubst $(SRC_MULIB_SCHED_DIR)/%.c, $(BUILD_MULIB_DIR)/sched/%.o, $(SOURCES_MU_SCHED))
OBJECTS_MU_STRING ?= $(patsubst $(SRC_MULIB_STRING_DIR)/%.c, $(BUILD_MULIB_DIR)/string/%.o, $(SOURCES_MU_STRING))
OBJECTS_MU_TERM ?= $(patsubst $(SRC_MULIB_TERM_DIR)/%.c, $(BUILD_MULIB_DIR)/term/%.o, $(SOURCES_MU_TERM))
OBJECTS_MU_UTILS ?= $(patsubst $(SRC_MULIB_UTILS_DIR)/%.c, $(BUILD_MULIB_DIR)/utils/%.o, $(SOURCES_MU_UTILS))


OBJECTS_MU_PLATFORM ?= $(patsubst $(SRC_MU_PLATFORM_DIR)/%.c, $(BUILD_MU_PLATFORM_DIR)/%.o, $(SOURCES_MU_PLATFORM))
OBJECTS_APP ?= $(patsubst $(SRC_APP_DIR)/%.c, $(BUILD_APP_DIR)/%.o, $(SOURCES_APP))

CFLAGS = -Wall -Werror -g -DMU_LOG_ENABLED

IFLAGS = -I $(INC_MULIB_COLLECTIONS_DIR) -I $(INC_MULIB_LOG_DIR) -I $(INC_MULIB_SCHED_DIR) -I $(INC_MULIB_STRING_DIR) -I $(INC_MULIB_TERM_DIR) -I $(INC_MULIB_UTILS_DIR) -I $(INC_MU_PLATFORM_DIR) -I $(INC_APP_DIR) 

#$(info    OBJECTS_MU_PLATFORM is $(OBJECTS_MU_PLATFORM))

all: $(OBJECTS_MU_COLLECTIONS) $(OBJECTS_MU_LOG) $(OBJECTS_MU_SCHED) $(OBJECTS_MU_STRING) $(OBJECTS_MU_TERM) $(OBJECTS_MU_UTILS) $(OBJECTS_MU_PLATFORM) $(OBJECTS_APP) main.o
	$(CC) $(CFLAGS) $(IFLAGS) $(OBJECTS_MU_COLLECTIONS) $(OBJECTS_MU_LOG) $(OBJECTS_MU_SCHED) $(OBJECTS_MU_STRING) $(OBJECTS_MU_TERM) $(OBJECTS_MU_UTILS) $(OBJECTS_MU_PLATFORM) $(OBJECTS_APP)  main.o -o $(APP_NAME)

run: all
	./$(APP_NAME)

clean:
	rm -rf $(BUILD_DIR)
	rm main.o

.c.o:
	$(CC) $(CFLAGS) $(IFLAGS) -c $(<) -o $@

#$(MODULES_USED): $(MODULES_DIR)/$(@)/src/%.c $(MODULES_DIR)/$(@)/inc/%.h
#	$(info    AAAAA into dir: is $(@D))
#	@mkdir -p $(BUILD_MULIB_DIR)/$(@)
#	$(CC) $(CFLAGS) $(IFLAGS) -c $(<) -o $@

$(BUILD_MULIB_DIR)/collections/%.o : $(SRC_MULIB_COLLECTIONS_DIR)/%.c $(INC_MULIB_COLLECTIONS_DIR)/*.h
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(IFLAGS) -c $(<) -o $@

$(BUILD_MULIB_DIR)/log/%.o : $(SRC_MULIB_LOG_DIR)/%.c $(INC_MULIB_LOG_DIR)/*.h
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(IFLAGS) -c $(<) -o $@

$(BUILD_MULIB_DIR)/sched/%.o : $(SRC_MULIB_SCHED_DIR)/%.c $(INC_MULIB_SCHED_DIR)/*.h
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(IFLAGS) -c $(<) -o $@

$(BUILD_MULIB_DIR)/string/%.o : $(SRC_MULIB_STRING_DIR)/%.c $(INC_MULIB_STRING_DIR)/*.h
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(IFLAGS) -c $(<) -o $@

$(BUILD_MULIB_DIR)/term/%.o : $(SRC_MULIB_TERM_DIR)/%.c $(INC_MULIB_TERM_DIR)/*.h
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(IFLAGS) -c $(<) -o $@

$(BUILD_MULIB_DIR)/utils/%.o : $(SRC_MULIB_UTILS_DIR)/%.c $(INC_MULIB_UTILS_DIR)/*.h
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(IFLAGS) -c $(<) -o $@

$(BUILD_MU_PLATFORM_DIR)/%.o: $(SRC_MU_PLATFORM_DIR)/%.c $(INC_MU_PLATFORM_DIR)/*.h
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(IFLAGS) -c $(<) -o $@

$(BUILD_APP_DIR)/%.o: $(SRC_APP_DIR)/%.c $(INC_APP_DIR)/*.h
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(IFLAGS) -c $(<) -o $@
