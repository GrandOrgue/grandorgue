add_option(-mmmx)
add_option(-msse)
add_option(-msse2)
add_option(-msse3)

add_option(-mstackrealign)

set(CMAKE_CXX_FLAGS_DEBUG "-g --coverage")
set(COVERAGE_EXCLUDES 'submodules/*' 'usr/*')



include(UsewxWidgets)
include_directories(${CMAKE_BINARY_DIR}/src/core/go_defs.h ${CMAKE_SOURCE_DIR}/src/core)

# Add here your tests files
set(go_test_lib
    # Test framework - Don't modify
    GOTestResult.cpp
    GOTestResultCollection.cpp
    GOTest.cpp
    GOTestCollection.cpp
    # Add here the individual tests
    GOTestWindchest.cpp
    GOTestSwitch.cpp

    # GOOrganModel
    GOTestOrganModel.cpp
)

add_library(GOTestLib STATIC ${go_test_lib})
find_package(cpptrace REQUIRED)

target_link_libraries(GOTestLib cpptrace::cpptrace)
target_link_libraries(GOTestLib GrandOrgueCore)
target_link_libraries(GOTestLib golib)

target_include_directories(GOTestLib PUBLIC ${CMAKE_SOURCE_DIR}/src/tests)
target_include_directories(GOTestLib PUBLIC ${CMAKE_SOURCE_DIR}/src/grandorgue)


add_option(-mmmx)
add_option(-msse)
add_option(-msse2)
add_option(-msse3)

add_option(-mstackrealign)

include(UsewxWidgets)
add_executable(GOTestExe GOTestExe.cpp)


target_link_libraries(GOTestExe GOTestLib)
target_link_libraries(GOTestExe golib)
target_include_directories(GOTestExe PUBLIC ${CMAKE_SOURCE_DIR}/src/tests)
BUILD_EXECUTABLE(GOTestExe)

add_test(NAME GOTestExe COMMAND GOTestExe)
include(CTest)