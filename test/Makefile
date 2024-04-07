# Main Makefile

# List of your configuration files
CONFIG_FILES := \
	make-mu-array.mk \
	make-mu-bisync.mk \
	make-mu-buf.mk \
	make-mu-jems.mk \
	make-mu-log.mk \
	make-mu-mqueue.mk \
	make-mu-queue.mk \
	make-mu-sched.mk \
	make-mu-spsc.mk \
	make-mu-str.mk \
	make-mu-task.mk

# Targets that should be forwarded to makefile_common.mk
COMMON_TARGETS := tests coverage clean

# Default target
all: $(CONFIG_FILES)

# Rule to process each configuration with make-shared.mk for the default build
$(CONFIG_FILES):
	@$(MAKE) -f make-shared.mk CONFIG_FILE=$@ $(MAKECMDGOALS)

# Special rule to handle common targets for all configurations
$(COMMON_TARGETS): $(CONFIG_FILES)

# Prevent Make from confusing common targets with actual files
.PHONY: all $(COMMON_TARGETS) $(CONFIG_FILES)

