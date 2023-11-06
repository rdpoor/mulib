 # compile / run / clean unit tests

 run_unit_tests:
	(cd mu_extras/test; make run_unit_tests)
	(cd mu_schedule/test; make run_unit_tests)
	(cd mu_string/test; make run_unit_tests)

clean:
	(cd mu_extras/test; make clean)
	(cd mu_schedule/test; make clean)
	(cd mu_string/test; make clean)

