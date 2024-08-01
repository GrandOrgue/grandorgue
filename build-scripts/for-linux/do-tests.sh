#!/bin/bash

# $1 - optional - [test | coverage | gcovr] - If nothing is provided,
# run all the tests steps.

set -e

STEP=$1
SCRIPT=""

TEST_SCRIPT="ctest -T test --verbose"
COVERAGE_SCRIPT="ctest -T coverage"
GCOVR_SCRIPT="gcovr -e 'submodules/*|usr/*'"

case $STEP in
    "test") 
        SCRIPT=$TEST_SCRIPT
    ;;
    "coverage")
        SCRIPT=$COVERAGE_SCRIPT
    ;;
    "gcovr")
        SCRIPT=$GCOVR_SCRIPT
    ;;
    "")
        SCRIPT="$TEST_SCRIPT ; $COVERAGE_SCRIPT ; $GCOVR_SCRIPT"
esac;

$SCRIPT
