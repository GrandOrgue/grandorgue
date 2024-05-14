# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

add_option(-mmmx)
add_option(-msse)
add_option(-msse2)
add_option(-msse3)

add_option(-mstackrealign)

include(UsewxWidgets)

include_directories(${CMAKE_BINARY_DIR}/src/core/go_defs.h ${CMAKE_SOURCE_DIR}/src/core)

if (WIN32)
   set_source_files_properties("${RESOURCEDIR}/GrandOrgue.rc" PROPERTIES GENERATED "YES")
   add_executable(GrandOrgueTool GOTool.cpp "${RESOURCEDIR}/GrandOrgue.rc")
   add_dependencies(GrandOrgueTool resources) # GrandOrgue.rc and GrandOrgue.manifest & GOIcon.ico referenced from GrandOrgue.rc
   add_linker_option(GrandOrgueTool large-address-aware)
else ()
   add_executable(GrandOrgueTool GOTool.cpp)
endif ()

BUILD_EXECUTABLE(GrandOrgueTool)
target_link_libraries(GrandOrgueTool GrandOrgueCore)

add_executable(GrandOrguePerfTest GOPerfTest.cpp)
BUILD_EXECUTABLE(GrandOrguePerfTest)
target_include_directories(GrandOrguePerfTest PUBLIC ${CMAKE_SOURCE_DIR}/src/grandorgue)
target_link_libraries(GrandOrguePerfTest golib)

add_custom_target(runperftest COMMAND GrandOrguePerfTest "${CMAKE_SOURCE_DIR}/tests" DEPENDS GrandOrguePerfTest)
