# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later
# (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

set(DEPLIST)

# ========================
# Generate icons from svg
# ========================

# Find ImageMagick
find_program(ImageMagick_convert_EXECUTABLE convert)

# Setup command to generate GOIcon.ico.
# That icon is used for two purposes:
# 1. Windows application icon (included from GrandOrgue.rc that is copied below and included from ${CMAKE_SOURCE_DIR}/src/grandorgue/CMakeLists.txt)
# 2. Conversion into .cpp and usage in GO code (in ${CMAKE_SOURCE_DIR}/src/images/CMakeLists.txt)
ADD_CUSTOM_COMMAND(OUTPUT "${RESOURCEDIR}/GOIcon.ico"
                     COMMAND ${ImageMagick_convert_EXECUTABLE} -background none -resize 32x32 "${CMAKE_SOURCE_DIR}/resources/GrandOrgue.svg" "${RESOURCEDIR}/GOIcon.ico" 
                     DEPENDS ${CMAKE_SOURCE_DIR}/resources/GrandOrgue.svg)
# Following files reference GOIcon.ico by a relative path, because of that they need to be placed near GOIcon.ico
ADD_CUSTOM_COMMAND(OUTPUT "${RESOURCEDIR}/GrandOrgue.rc"
                   COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/resources/GrandOrgue.rc" "${RESOURCEDIR}/GrandOrgue.rc"
                   DEPENDS "${CMAKE_SOURCE_DIR}/resources/GrandOrgue.rc")
ADD_CUSTOM_COMMAND(OUTPUT "${RESOURCEDIR}/GrandOrgue.manifest"
                   COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/resources/GrandOrgue.manifest" "${RESOURCEDIR}/GrandOrgue.manifest"
                   DEPENDS "${CMAKE_SOURCE_DIR}/resources/GrandOrgue.manifest")
# Without that instuctions on building files above doesn't being added to Makefile
LIST(APPEND DEPLIST "${RESOURCEDIR}/GOIcon.ico" "${RESOURCEDIR}/GrandOrgue.rc" "${RESOURCEDIR}/GrandOrgue.manifest" ${GENERATED_ICONS})

# Configure generation of icons & create directory for generated .png icons
# Directory is named GrandOrgue.iconset because iconutil (for MacOS) requires such name
# Icons from this directory will be included into linux package (with renaming) and into MacOS icons (assembled into archive via iconutil)
set(ICON_SIZES 16 32 64 128 256 512 1024)
set(GENERATED_ICONS_DIR "${RESOURCEDIR}/GrandOrgue.iconset" )
ADD_CUSTOM_COMMAND(OUTPUT ${GENERATED_ICONS_DIR} COMMAND ${CMAKE_COMMAND} -E make_directory ${GENERATED_ICONS_DIR})

# Setup commands to generate .png icons of different sizes
# GENERATED_ICONS contains paths to generated icons
set(GENERATED_ICONS)
foreach(ICON_SIZE IN ITEMS ${ICON_SIZES})
  LIST(APPEND GENERATED_ICONS "${GENERATED_ICONS_DIR}/icon_${ICON_SIZE}x${ICON_SIZE}.png" )
  ADD_CUSTOM_COMMAND(OUTPUT "${GENERATED_ICONS_DIR}/icon_${ICON_SIZE}x${ICON_SIZE}.png" 
                     COMMAND ${ImageMagick_convert_EXECUTABLE} -background none -resize ${ICON_SIZE}x${ICON_SIZE} "${CMAKE_SOURCE_DIR}/resources/GrandOrgue.svg" "${GENERATED_ICONS_DIR}/icon_${ICON_SIZE}x${ICON_SIZE}.png"
                     DEPENDS ${CMAKE_SOURCE_DIR}/resources/GrandOrgue.svg ${GENERATED_ICONS_DIR})
endforeach()

# ========================
# Install resources
# ========================
function(add_man_page EXECUTABLE_NAME)
  ADD_CUSTOM_COMMAND(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_NAME}.1" COMMAND ${XSLTPROC} -o "${CMAKE_CURRENT_BINARY_DIR}/" ${DOCBOOK_PATH}/manpages/docbook.xsl "${CMAKE_CURRENT_SOURCE_DIR}/${EXECUTABLE_NAME}Man.xml" DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${EXECUTABLE_NAME}Man.xml)
  LIST(APPEND DEPLIST "${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_NAME}.1")
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_NAME}.1" DESTINATION share/man/man1)
  set(DEPLIST ${DEPLIST} PARENT_SCOPE)
endfunction()

if (APPLE)
   ADD_CUSTOM_COMMAND(OUTPUT "${RESOURCEDIR}/GrandOrgue.icns" COMMAND iconutil -c icns ${GENERATED_ICONS_DIR} DEPENDS ${GENERATED_ICONS})
   INSTALL(FILES "${RESOURCEDIR}/GrandOrgue.icns" DESTINATION "${RESOURCEINSTDIR}")
   LIST(APPEND DEPLIST "${RESOURCEDIR}/GrandOrgue.icns" ${GENERATED_ICONS})
elseif(UNIX)
  # Install generated icons for linux 
  LIST(APPEND DEPLIST ${GENERATED_ICONS})
  foreach(ICON_SIZE IN ITEMS ${ICON_SIZES})
    INSTALL(FILES "${GENERATED_ICONS_DIR}/icon_${ICON_SIZE}x${ICON_SIZE}.png" RENAME GrandOrgue.png DESTINATION share/icons/hicolor/${ICON_SIZE}x${ICON_SIZE}/apps)
  endforeach()

  install(FILES grandorgue.xml DESTINATION share/mime/packages)

  if (GETTEXT_FOUND)
    ADD_CUSTOM_COMMAND(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/GrandOrgue.desktop" COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} --desktop --template ${CMAKE_CURRENT_SOURCE_DIR}/GrandOrgue.desktop.in -d ${CMAKE_SOURCE_DIR}/po -o "${CMAKE_CURRENT_BINARY_DIR}/GrandOrgue.desktop" DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/GrandOrgue.desktop.in)
  else()
    ADD_CUSTOM_COMMAND(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/GrandOrgue.desktop" COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/GrandOrgue.desktop.in "${CMAKE_CURRENT_BINARY_DIR}/GrandOrgue.desktop" DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/GrandOrgue.desktop.in)
  endif()
  LIST(APPEND DEPLIST "${CMAKE_CURRENT_BINARY_DIR}/GrandOrgue.desktop")
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/GrandOrgue.desktop" DESTINATION share/applications)

  if(XSLTPROC AND DOCBOOK_PATH)
    add_man_page(GrandOrgue)
    add_man_page(GrandOrguePerfTest)
    add_man_page(GrandOrgueTool)
  else()
    MESSAGE(STATUS "Not build manpage - some programs are missing")
  endif()

  string(TIMESTAMP TODAY "%Y-%m-%d")
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/GrandOrgue.appdata.xml.in" "${CMAKE_CURRENT_BINARY_DIR}/GrandOrgue.appdata.xml/GrandOrgue.appdata.xml")
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/GrandOrgue.appdata.xml/GrandOrgue.appdata.xml" DESTINATION share/metainfo)
endif()

message(STATUS "DEPLIST=${DEPLIST}")

ADD_CUSTOM_TARGET(resources ALL DEPENDS ${DEPLIST})
