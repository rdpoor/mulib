 # compile / run / clean unit tests

unit_tests:
	(cd mu_extras/test; make unit_tests)
	(cd mu_schedule/test; make unit_tests)
	(cd mu_string/test; make unit_tests)

coverage_tests:
	(cd mu_extras/test; make coverage_tests)
	(cd mu_schedule/test; make coverage_tests)
	(cd mu_string/test; make coverage_tests)

clean:
	(cd mu_extras/test; make clean)
	(cd mu_schedule/test; make clean)
	(cd mu_string/test; make clean)

