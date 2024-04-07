# TODO for test directory

Each module needs its own obj and bin directory so only the required object
files are linked in.

Similarly for coverage: each module needs a separate directory (else the 
coverage directory gets over-written by the last `make coverage` command).

Finish tests for
* mu_mqueue
* mu_sched

An implmeentation must provide mu_platform.[ch] that defines a few compile
time constants (for scheduler) and time functions (mu_time_now(), ...)

