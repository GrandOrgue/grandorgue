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

function(GET_MO_DIR resultdir resultinstdir lang)
  IF (APPLE)
    SET(${resultdir} ${RESOURCEDIR}/${lang}.lproj PARENT_SCOPE)
    SET(${resultinstdir} ${RESOURCEINSTDIR}/${lang}.lproj PARENT_SCOPE)
  ELSE()
    SET(${resultdir} ${CMAKE_BINARY_DIR}/share/locale/${lang}/LC_MESSAGES PARENT_SCOPE)
    SET(${resultinstdir} share/locale/${lang}/LC_MESSAGES PARENT_SCOPE)
  ENDIF()
endfunction()

function(GETTEXT_UPDATE_POT target _potFile pattern)
  file(GLOB_RECURSE SRC_FILES ${pattern})
  set(_lst)
  foreach(FILENAME ${SRC_FILES})
    set(_lst "${_lst}${FILENAME}\n") 
  endforeach(FILENAME)
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${_potFile}.src_lst "${_lst}")
  ADD_CUSTOM_TARGET(${target} COMMAND ${GETTEXT_XGETTEXT_EXECUTABLE} -o ${_potFile} --files-from ${CMAKE_CURRENT_BINARY_DIR}/${_potFile}.src_lst ${ARGN})
endfunction()

function(GETTEXT_MERGE_PO target _potFile)
	SET(_cmds)
	FOREACH (_currentPoFile ${ARGN})
		SET(_cmds ${_cmds} COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} --update --backup=none -s ${_currentPoFile} ${_potFile} -v )
	ENDFOREACH (_currentPoFile )
	ADD_CUSTOM_TARGET(${target} ${_cmds})
endfunction()

function(GETTEXT_INSTALL_PO basename)
  SET(_moFiles)
  SET(_addToAll)
  FOREACH (_currentPoFile ${ARGN})
    GET_FILENAME_COMPONENT(_lang ${_currentPoFile} NAME_WE)
    GET_MO_DIR(_moDir _moInstDir ${_lang})
    SET(_moFile ${_moDir}/${basename}.mo)
    SET(_moFiles ${_moFiles} ${_moFile})
    file(MAKE_DIRECTORY ${_moDir})

    ADD_CUSTOM_COMMAND(OUTPUT ${_moFile} COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} -o ${_moFile} ${_currentPoFile} DEPENDS ${_currentPoFile})
    IF (GETTEXT_FOUND)
      INSTALL(FILES ${_moFile} DESTINATION ${_moInstDir} COMPONENT resources)
    ENDIF()
  ENDFOREACH (_currentPoFile )

  ADD_CUSTOM_TARGET(translations ALL DEPENDS ${_moFiles})
endfunction()

