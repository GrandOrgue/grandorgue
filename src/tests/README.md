* Add your tests files (cpp) in CMakeLists
* To test locally, run the build-scripts/for-linux/build-for-tests.sh or
  add the -DGO_BUILD_TESTING=ON option to your custom cmake.
* Run ctest in the build directory.

## Running GOTestExe selectively

`GOTestExe [options] [pattern...]`

Options:

* `--perf-only` - run only the performance tests
* `--no-perf` - run only the functional tests
* `--list` - print the names of the selected test classes and tests and exit
  without running them
* `--timing-top N` - how many of the slowest tests to show in the timing
  summary (20 by default, 0 means all)
* `--help` - show the usage

Patterns select the tests to run. A pattern is `ClassRegex` or
`ClassRegex/TestRegex`, where each part is an ECMAScript regular expression
searched in the class name or in the test name (so `Reader` matches any class
containing `Reader`; use `^...$` for the exact match):

* A pattern without `/` selects whole test classes.
* A pattern with `/` selects only the tests matching `TestRegex` from the
  classes matching `ClassRegex`.
* A pattern starting with `!` excludes instead of includes. If there are no
  include patterns then everything except the excluded is run.
* Several patterns may be given as separate arguments. A comma has no special
  meaning and is a part of the regular expression.
* Quote the patterns in the shell (`!`, `|`, `*` and `$` are special for it).

The names printed by `--list` and in the timing summary can be used as patterns
(better with `^` and `$`).

Examples:

    GOTestExe --list
    GOTestExe GOTestOrganReader
    GOTestExe 'GOTestSoundOrganEngine/TestSetUsedTransitions'
    GOTestExe Fader 'Resample/Linear'
    GOTestExe --no-perf '!Stress'

Exit codes: 0 - all selected tests passed, 1 - nothing matched the patterns,
2 - some tests failed, 3 - wrong arguments.

At the end of a run the tool prints a timing summary: all test classes and the
slowest tests sorted by the elapsed time.

## Writing tests

Run every test method of a test class from `run()` via `GO_RUN_TEST(TestFoo())`
(see `GOTestScope.h`) - this logs its time and makes it selectable by the
patterns. `run()` must contain nothing but `GO_RUN_TEST` calls, because in the
`--list` mode and before running a class (to skip the classes that have no
selected tests) it is called without `setUp()`. Put the preparation (banners,
clearing the state) to `setUp()` and `tearDown()`. A check that must report a
failure (like `ReportFailedTests()`) stays in `run()`, because an exception
thrown from `tearDown()` is not counted as a test failure.
