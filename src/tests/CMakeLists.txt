# include(UsewxWidgets

add_option(-mmmx)
add_option(-msse)
add_option(-msse2)
add_option(-msse3)

add_option(-mstackrealign)
include(UsewxWidgets)

include_directories(${CMAKE_BINARY_DIR}/src/core/go_defs.h ${CMAKE_SOURCE_DIR}/src/core)

add_executable(GOTestLib GOTestLib.cpp)

target_include_directories(GOTestLib PUBLIC ${CMAKE_SOURCE_DIR}/src/grandorgue)

target_link_libraries(GOTestLib golib)

add_test(NAME GOTestLib COMMAND GOTestLib)

enable_testing()
BUILD_EXECUTABLE(GOTestLib)
