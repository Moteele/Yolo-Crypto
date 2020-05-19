#!/bin/bash
# Generates html structure with code coverage, if lcov is installed.
# To see it, open index.html in the output folder

# is lcov installed?
lcov -V;
if [[ $? != 0 ]]; then
	echo "Please install 'lcov' for html output";
else
	lcov --capture --directory ./ --output-file coverage.info && genhtml coverage.info --output-directory coverage &&
	(echo "==============================================================="
	echo " * html files written to ${PWD}/coverage"
	echo "===============================================================")
fi
