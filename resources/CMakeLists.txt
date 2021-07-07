# GrandOrgue - free pipe organ simulator
# 
# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

set(DEPLIST)

# ========================
# Generate icons from svg
# ========================

# Find ImageMagick
find_package(ImageMagick COMPONENTS convert)

# Configure generation of icons & create directory for generated icons
set(ICON_SIZES 16 32 64 128 256 512 1024)
set(GENERATED_ICONS_DIR "${RESOURCEDIR}/generated" )
ADD_CUSTOM_COMMAND(OUTPUT ${GENERATED_ICONS_DIR} COMMAND ${CMAKE_COMMAND} -E make_directory ${GENERATED_ICONS_DIR})

# Setup commands to generate icons
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
    ADD_CUSTOM_COMMAND(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/GrandOrgue.1" COMMAND ${XSLTPROC} -o "${CMAKE_CURRENT_BINARY_DIR}/" ${DOCBOOK_PATH}/manpages/docbook.xsl "${CMAKE_CURRENT_SOURCE_DIR}/man.xml" DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/man.xml)
    LIST(APPEND DEPLIST "${CMAKE_CURRENT_BINARY_DIR}/GrandOrgue.1")
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/GrandOrgue.1" DESTINATION share/man/man1)
  else()
    MESSAGE(STATUS "Not build manpage - some programs are missing")
  endif()

  install(FILES GrandOrgue.appdata.xml DESTINATION share/metainfo)

endif()

ADD_CUSTOM_TARGET(resources ALL DEPENDS ${DEPLIST})
