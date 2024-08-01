#!/bin/bash

# $1 - optional - [test | coverage | gcovr] - If nothing is provided,
# run all the tests steps.

set -e

STEP=$1

TEST_SCRIPT="ctest -T test --verbose"
COVERAGE_SCRIPT="ctest -T coverage"
GCOVR_SCRIPT="gcovr -e 'submodules/*|usr/*'"

case $STEP in
    "test") 
        $TEST_SCRIPT
    ;;
    "coverage")
        $COVERAGE_SCRIPT
    ;;
    "gcovr")
        $GCOVR_SCRIPT
    ;;
    "")
        $TEST_SCRIPT ; $COVERAGE_SCRIPT ; $GCOVR_SCRIPT
esac;


