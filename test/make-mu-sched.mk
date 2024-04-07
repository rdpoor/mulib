# make-mu-sched.mk

SRC_FILES := \
    $(SRC_DIR)/mu_queue.c \
    $(SRC_DIR)/mu_sched.c \
    $(SRC_DIR)/mu_spsc.c \
    $(SRC_DIR)/mu_task.c

TEST_FILES := \
    $(TEST_DIR)/test_mu_sched.c

OTHER_FLAGS := -I$(TEST_DIR)
