#!/bin/bash

# Generates flat profiling statistics

j=4

## server test
#make -j${j} profile-test-server && ./test-server
#flags=`make profile-print-OPTIONAL | grep "OPTIONAL=-" | sed -e 's/ /_/g'`
#gprof -p test-server gmon.out > analysis_test-server_${flags}.out
#
## client test
#make -j${j} profile-test-client && ./test-client
#flags=`make profile-print-OPTIONAL | grep "OPTIONAL=-" | sed -e 's/ /_/g'`
#gprof -p test-client > analysis_test-client_${flags}.out

## util test
#make -j${j} profile-test-util && ./test-util
#flags=`make profile-print-OPTIONAL | grep "OPTIONAL=-" | sed -e 's/ /_/g'`
#gprof -p test-util > analysis_test-util_${flags}.out
#echo "Written to analysis_test-util_${flags}.out"

# Functions test
# compile with profiling flags and run
make -j${j} profile-test-functions && ./test-functions
# get OPTIONAL flags
flags=`make profile-print-OPTIONAL | grep "OPTIONAL=-" | sed -e 's/ /_/g'`
gprof -p test-functions > analysis_test-functions_${flags}.out

## client account creation test
#make -j${j} profile-all && ./run.sh
#flags=`make profile-print-OPTIONAL | grep "OPTIONAL=-" | sed -e 's/ /_/g'`
#gprof -p clientApp > analysis_clientApp_${flags}.out
#echo "Written to analysis_clientApp_${flags}.out"
