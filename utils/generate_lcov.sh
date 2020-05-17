#!/bin/bash

lcov -V;
if [[ $? != 0 ]]; then
	echo "Please install 'lcov' for html output";
else
	lcov --capture --directory ./ --output-file coverage.info && genhtml coverage.info --output-directory coverage &&
	(echo "==============================================================="
	echo " * html files written to ${PWD}/coverage"
	echo "===============================================================")
fi
