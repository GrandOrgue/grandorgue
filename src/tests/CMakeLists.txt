add_option(-mmmx)
add_option(-msse)
add_option(-msse2)
add_option(-msse3)

add_option(-mstackrealign)

include(UsewxWidgets)
include_directories(${CMAKE_BINARY_DIR}/src/core/go_defs.h ${CMAKE_SOURCE_DIR}/src/core)

add_subdirectory(common)
add_subdirectory(testing)

add_executable(GOTestExe GOTestExe.cpp)


target_link_libraries(GOTestExe GOTestLib)
target_link_libraries(GOTestExe golib)
target_link_libraries(GOTestExe GOTests)
target_include_directories(GOTestExe PUBLIC ${CMAKE_SOURCE_DIR}/src/tests)
target_include_directories(GOTestExe PUBLIC ${CMAKE_SOURCE_DIR}/src/tests/common)
target_include_directories(GOTestExe PUBLIC ${CMAKE_SOURCE_DIR}/src/tests/testing)
target_include_directories(GOTestExe PUBLIC ${CMAKE_SOURCE_DIR}/src/tests/testing/model)
target_include_directories(GOTests PUBLIC ${CMAKE_SOURCE_DIR}/src/tests/common)
BUILD_EXECUTABLE(GOTestExe)

add_test(NAME GOTestExe COMMAND GOTestExe)
include(CTest)