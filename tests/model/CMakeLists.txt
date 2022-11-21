include(UsewxWidgets)
add_executable(windchest_test windchest_test.cpp)

target_link_directories(
    windchest_test
    PUBLIC ${PROJECT_SOURCE_DIR}/src
    ${EXTRA_LIB_DIRS}
)

target_link_libraries(windchest_test ${PROJECT_SOURCE_DIR}/src/grandorgue/model/GOWindchest.h)

add_test(NAME windchest_test COMMAND "windchest_test")