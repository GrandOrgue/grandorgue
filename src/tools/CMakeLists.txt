# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
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
target_link_libraries(GrandOrgueTool golib)

add_executable(perftest EXCLUDE_FROM_ALL perftest.cpp)
target_link_libraries(perftest golib)

add_custom_target(runperftest COMMAND perftest "${CMAKE_SOURCE_DIR}/tests" DEPENDS perftest)
