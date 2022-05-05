.PHONY: all clean

# ==========
# Define the location of key directories

PROJECT_PATH := ..

SOURCE_DIR := $(PROJECT_PATH)/mulib
CORE_DIR := $(SOURCE_DIR)/core
EXTRAS_DIR := $(SOURCE_DIR)/extras
LABORATORY_DIR := $(SOURCE_DIR)/laboratory

TEST_DIR := $(PROJECT_PATH)/mulib_tests
CORE_TEST_DIR := $(TEST_DIR)/core
EXTRAS_TEST_DIR := $(TEST_DIR)/extras
LABORATORY_TEST_DIR := $(TEST_DIR)/laboratory

PLATFORM_DIR := $(PROJECT_PATH)/tools/platform

# One very flat build directory for everything...
BUILD_DIR := $(PROJECT_PATH)/build

# ==========
# Define header and source files

CORE_HEADERS := $(wildcard $(CORE_DIR)/*.h)
EXTRAS_HEADERS := $(wildcard $(EXTRAS_DIR)/*.h)
LABORATORY_HEADERS := $(wildcard $(LABORATORY_DIR)/*.h)

CORE_SOURCES := $(wildcard $(CORE_DIR)/*.c)
EXTRAS_SOURCES := $(wildcard $(EXTRAS_DIR)/*.c)
LABORATORY_SOURCES := $(wildcard $(LABORATORY_DIR)/*.c)

CORE_TEST_SOURCES := $(wildcard $(CORE_TEST_DIR)/*.c)
EXTRAS_TEST_SOURCES := $(wildcard $(EXTRAS_TEST_DIR)/*.c)
LABORATORY_TEST_SOURCES := $(wildcard $(LABORATORY_TEST_DIR)/*.c)

PLATFORM_HEADERS := $(wildcard $(PLATFORM_DIR)/*.h)
PLATFORM_SOURCES := $(wildcard $(PLATFORM_DIR)/*.c)

# ==========
# Define object files

CORE_OBJECTS ?= $(patsubst $(CORE_DIR)/%.c, \
     $(BULID_DIR)%.o, \
     $(CORE_SOURCES))
EXTRAS_OBJECTS ?= $(patsubst $(EXTRAS_DIR)/%.c, \
     $(BUILD_DIR)%.o, \
     $(EXTRAS_SOURCES))
LABORATORY_OBJECTS ?= $(patsubst $(LABORATORY_DIR)/%.c, \
     $(BULID_DIR)%.o, \
     $(LABORATORY_SOURCES))

CORE_TEST_OBJECTS ?= $(patsubst $(CORE_TEST_DIR)/%.c, \
     $(BULID_DIR)%.o, \
     $(CORE_TEST_SOURCES))
EXTRAS_TEST_OBJECTS ?= $(patsubst $(EXTRAS_TEST_DIR)/%.c, \
     $(BUILD_DIR)%.o, \
     $(EXTRAS_TEST_SOURCES))
LABORATORY_TEST_OBJECTS ?= $(patsubst $(LABORATORY_TEST_DIR)/%.c, \
     $(BULID_DIR)%.o, \
     $(LABORATORY_TEST_SOURCES))

PLATFORM_OBJECTS ?= $(patsubst $(PLATFORM_DIR)/%.c, \
     $(BULID_DIR)/%.o, \
	 $(PLATFORM_SOURCES))

# ==========
# settings

CFLAGS = -Wall -Werror -g

IFLAGS = -I $(SOURCE_DIR)


# ==========
# Rules...

core: $(BUILD_DIR) $(CORE_OBJECTS)
	$(CC) $(CFLAGS) $(IFLAGS) -c $(<) -o $@

extras: $(BUILD_DIR) $(EXTRAS_OBJECTS)

laboratory: $(BUILD_DIR) $(LABORATORY_OBJECTS)

all: $(BUILD_DIR) $(CORE_OBJECTS)

$(BUILD_DIR) :
	mkdir -p $(BUILD_DIR)

.c.o:
	$(CC) $(CFLAGS) $(IFLAGS) -c $(<) -o $@

test: $(BUILD_DIR)
	echo $(CORE_OBJECTS)

clean:
	rm -rf $(BUILD_DIR)


# SANDBOX_PATH := /Users/r/Projects/make_test
# TARBALLS_DIR := $(SANDBOX_PATH)/tarballs
# INSTALL_DIR := $(SANDBOX_PATH)/usr
#
# RUBY_TARGET := $(INSTALL_DIR)/bin/ruby
# RUBY_SOURCES := $(TARBALLS_DIR)/ruby-2.0.0-p0
# RUBY_TARBALL := $(TARBALLS_DIR)/ruby-2.0.0-p0.tar.gz
# RUBY_DISTRO := http://ftp.ruby-lang.org/pub/ruby/2.0/ruby-2.0.0-p0.tar.gz
#
# $(TARBALLS_DIR) :
# 	mkdir -p $(TARBALLS_DIR)
#
# $(INSTALL_DIR) :
# 	mkdir -p $(INSTALL_DIR)
#
# # ================================================================
# # ==== ruby
# ruby : $(RUBY_TARGET)
#
# # see http://collectiveidea.com/blog/archives/2011/10/31/install-ruby-193-with-libyaml-on-centos/
# $(RUBY_TARGET) : $(RUBY_SOURCES)
# 	(cd $< ; \
# 	 ./configure --prefix=$(SANDBOX_PATH)/usr --disable-install-doc ; \
# 	 make  ; make install )
#
# $(RUBY_SOURCES) : $(RUBY_TARBALL) | $(INSTALL_DIR)
# 	(cd $(TARBALLS_DIR) ; tar xf $(RUBY_TARBALL) )
#
# $(RUBY_TARBALL) : | $(TARBALLS_DIR)
# 	(cd $(TARBALL_DIR) ; curl $(RUBY_DISTRO) -o $(RUBY_TARBALL))
