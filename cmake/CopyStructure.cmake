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

function(COPY_STRUCTURE TARGET DIRNAME COMPONENT)
  SET(DIR "${RESOURCEDIR}/${DIRNAME}")
  SET(INSTDIR "${RESOURCEINSTDIR}/${DIRNAME}")

  SET(SRCDIR ${CMAKE_CURRENT_SOURCE_DIR})
  file(MAKE_DIRECTORY ${DIR})  
  file(GLOB_RECURSE SRCFILES RELATIVE ${SRCDIR} ${SRCDIR}/*.organ ${SRCDIR}/*.wav ${SRCDIR}/*.gif ${SRCDIR}/*.jpg ${SRCDIR}/*.png ${SRCDIR}/*.wv ${SRCDIR}/*.orgue)
  SET(_FILELIST)
  FOREACH(_realFile ${SRCFILES})
    STRING(REPLACE "#" "s" _currentFile "${_realFile}" )
    STRING(REPLACE "#" "*" _escapedRealFile "${_realFile}" )
    if ("${_currentFile}" MATCHES "\\.organ")
      ADD_CUSTOM_COMMAND(OUTPUT "${DIR}/${_currentFile}" COMMAND ${CMAKE_COMMAND} -Dinfile="${SRCDIR}/${_realFile}" -Doutfile="${DIR}/${_currentFile}" -P "${CMAKE_SOURCE_DIR}/cmake/AdaptFilenames.cmake"  DEPENDS "${SRCDIR}/${_esacpedRealFile}")
    else()
      ADD_CUSTOM_COMMAND(OUTPUT "${DIR}/${_currentFile}" COMMAND ${CMAKE_COMMAND} -E copy "${SRCDIR}/${_realFile}" "${DIR}/${_currentFile}" DEPENDS "${SRCDIR}/${_escapedRealFile}")
    endif()
    get_filename_component(_dir ${_currentFile} PATH)
    INSTALL(FILES ${DIR}/${_currentFile} DESTINATION ${INSTDIR}/${_dir} COMPONENT ${COMPONENT})
    SET(_FILELIST ${_FILELIST} ${DIR}/${_currentFile})
  ENDFOREACH()
  ADD_CUSTOM_TARGET(${TARGET} ALL DEPENDS ${_FILELIST})
endfunction()
