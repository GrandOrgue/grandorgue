find_package(cpptrace REQUIRED)
set(go_test_common
    # Test framework - Don't modify
    GOTestResult.cpp
    GOTestResultCollection.cpp
    GOTest.cpp
    GOTestCollection.cpp
)

add_library(GOTestLib STATIC ${go_test_common})
target_link_libraries(GOTestLib cpptrace::cpptrace)
target_link_libraries(GOTestLib GrandOrgueCore)
target_link_libraries(GOTestLib golib)

target_include_directories(GOTestLib PUBLIC ${CMAKE_SOURCE_DIR}/src/tests)
target_include_directories(GOTestLib PUBLIC ${CMAKE_SOURCE_DIR}/src/grandorgue)