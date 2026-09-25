* Add your tests files (cpp) in CMakeLists
* To test locally, run the build-scripts/for-linux/build-for-tests.sh or
  add the -DTEST_BUILDING=ON option to your custom cmake.
* Run ctest in the build directory.

## MIDI late discovery

`GOTestMidiSystem` uses simulated ports and a headless wxWidgets event loop, so
it needs no MIDI hardware. It verifies that configured input and output ports
which appear after startup open with their saved mappings, that failed opens are
retried if their address changes, and that discovery stops after success.
