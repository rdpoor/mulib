# Define a list of directories
TEST_DIRS = \
	test_mu_array \
	test_mu_bbuf \
	test_mu_bisync \
	test_mu_bvec \
	test_mu_jems \
	test_mu_log \
	test_mu_mqueue \
	test_mu_queue \
	test_mu_sched \
	test_mu_spsc \
	test_mu_task

# Define a function to iterate over directories and run a command
run-command:
	@for dir in $(TEST_DIRS); do \
		echo Entering $$dir...; \
		$(MAKE) -C $$dir $(CMD); \
	done

# Targets that utilize the function
.PHONY: tests coverage clean

tests coverage clean:
	$(MAKE) run-command CMD=$@
