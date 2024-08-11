# Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

message(STATUS "wxMaterialDesign Art Provider configuration")

include(UsewxWidgets)
INCLUDE (CheckIncludeFileCXX)


set(wxMaterial_Source "${WX_MATERIAL_DESIGN_SRC_DIR}/wxMaterialDesignArtProvider.cpp")

include_directories(WX_MATERIAL_DESIGN_SRC_DIR)

add_library(wxMaterial STATIC ${wxMaterial_Source})
target_link_libraries(wxMaterial ${wxWidgets_LIBRARIES})


# find_package(wxWidgets REQUIRED wxMaterial)

message(STATUS "============================================================================")
