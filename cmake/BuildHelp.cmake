# GrandOrgue - free pipe organ simulator
# 
# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2015 GrandOrgue contributors (see AUTHORS)
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

function(BUILD_HELPFILE xmlfile VARIANT)
  SET(HELPDIR "${RESOURCEDIR}/help")
  SET(HELPINSTDIR "${RESOURCEINSTDIR}/help")

  SET(BUILDDIR ${CMAKE_CURRENT_BINARY_DIR}/help${VARIANT})
  SET(SRCDIR ${CMAKE_CURRENT_SOURCE_DIR})
  SET(IMGDIR ${SRCDIR}/images)
  SET(IMGLOCDIR ${SRCDIR}/images-${VARIANT})
  FILE(GLOB_RECURSE IMAGES RELATIVE ${IMGDIR} ${IMGDIR}/*.gif ${IMGDIR}/*.jpg ${IMGDIR}/*.png)
  file(MAKE_DIRECTORY ${BUILDDIR})
  file(MAKE_DIRECTORY ${HELPDIR})

  SET(_ImgList)
  FOREACH(IMG_NAME ${IMAGES})
    if(EXISTS ${IMGLOCDIR}/${IMG_NAME})
      set(_Img ${IMGLOCDIR}/${IMG_NAME})
    else()
      set(_Img ${IMGDIR}/${IMG_NAME})
    endif()
    ADD_CUSTOM_COMMAND(OUTPUT ${BUILDDIR}/images/${IMG_NAME} COMMAND ${CMAKE_COMMAND} -E copy ${_Img} ${BUILDDIR}/images/${IMG_NAME} DEPENDS ${_Img})
    SET(_ImgList ${_ImgList} ${BUILDDIR}/images/${IMG_NAME})
  ENDFOREACH()
  
  ADD_CUSTOM_COMMAND(OUTPUT ${BUILDDIR}/GrandOrgue.hhp COMMAND ${XSLTPROC}
    ARGS --path ${DOCBOOK_PATH}/htmlhelp ${SRCDIR}/grandorgue.xsl ${xmlfile}
    WORKING_DIRECTORY ${BUILDDIR} DEPENDS ${SRCDIR}/grandorgue.xsl ${xmlfile})

  ADD_CUSTOM_COMMAND(OUTPUT ${HELPDIR}/GrandOrgue${VARIANT}.htb COMMAND ${ZIP} 
    ARGS -r --filesync ${HELPDIR}/GrandOrgue${VARIANT}.htb * WORKING_DIRECTORY ${BUILDDIR}
    DEPENDS ${_ImgList} ${BUILDDIR}/GrandOrgue.hhp)

  INSTALL(FILES ${HELPDIR}/GrandOrgue${VARIANT}.htb DESTINATION ${HELPINSTDIR})
  ADD_CUSTOM_TARGET(build-helpfile${VARIANT} DEPENDS ${HELPDIR}/GrandOrgue${VARIANT}.htb)

  IF (NOT TARGET build-help)
      ADD_CUSTOM_TARGET(build-help ALL)
  ENDIF ()
  ADD_DEPENDENCIES(build-help build-helpfile${VARIANT})
endfunction()
