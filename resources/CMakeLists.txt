# GrandOrgue - free pipe organ simulator
# 
# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2017 GrandOrgue contributors (see AUTHORS)
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

if (APPLE)
   ADD_CUSTOM_COMMAND(OUTPUT "${RESOURCEDIR}/GrandOrgue.icns" COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/resources/GrandOrgue.icns "${RESOURCEDIR}/GrandOrgue.icns" DEPENDS ${CMAKE_SOURCE_DIR}/resources/GrandOrgue.icns)
   INSTALL(FILES "${RESOURCEDIR}/GrandOrgue.icns" DESTINATION "${RESOURCEINSTDIR}")
   LIST(APPEND DEPLIST "${RESOURCEDIR}/GrandOrgue.icns")
elseif(UNIX)
  install(FILES GrandOrgue.png DESTINATION share/pixmaps)
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

  install(FILES GrandOrgue.appdata.xml DESTINATION share/appdata)

endif()

ADD_CUSTOM_TARGET(resources ALL DEPENDS ${DEPLIST})
