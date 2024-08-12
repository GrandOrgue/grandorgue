# Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

message(STATUS "wxMaterialDesign Art Provider configuration")

include(UsewxWidgets)
INCLUDE (CheckIncludeFileCXX)

set(wxMaterial_src 
${WX_MATERIAL_DESIGN_SRC_DIR}/wxAwesomeBrandsArt.cpp
${WX_MATERIAL_DESIGN_SRC_DIR}/wxAwesomeRegularArt.cpp
${WX_MATERIAL_DESIGN_SRC_DIR}/wxAwesomeSolidArt.cpp
${WX_MATERIAL_DESIGN_SRC_DIR}/wxFluentuiFilledArt.cpp
${WX_MATERIAL_DESIGN_SRC_DIR}/wxMaterialFilledArt.cpp
${WX_MATERIAL_DESIGN_SRC_DIR}/wxFluentuiRegularArt.cpp
${WX_MATERIAL_DESIGN_SRC_DIR}/wxMaterialDesignArtProvider.cpp
${WX_MATERIAL_DESIGN_SRC_DIR}/wxMaterialOutlinedArt.cpp
${WX_MATERIAL_DESIGN_SRC_DIR}/wxMaterialRoundArt.cpp
${WX_MATERIAL_DESIGN_SRC_DIR}/wxMaterialSharpArt.cpp
${WX_MATERIAL_DESIGN_SRC_DIR}/wxMaterialTwoToneArt.cpp
${WX_MATERIAL_DESIGN_SRC_DIR}/wxSimpleIconsArt.cpp
)
set(wxMaterial_Source "${WX_MATERIAL_DESIGN_SRC_DIR}/wxMaterialDesignArtProvider.cpp")

include_directories(WX_MATERIAL_DESIGN_SRC_DIR)

add_library(wxMaterial STATIC ${wxMaterial_src})
target_link_libraries(wxMaterial ${wxWidgets_LIBRARIES})


# find_package(wxWidgets REQUIRED wxMaterial)

message(STATUS "============================================================================")
