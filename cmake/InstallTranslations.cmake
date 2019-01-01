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

include(${CMAKE_SOURCE_DIR}/cmake/GetText.cmake)

function(InstallTranslations basename)
  set(moFiles)
  foreach(dir ${ARGN})
    file(GLOB_RECURSE _tmpFiles ${dir}/*.mo)
    set(moFiles ${moFiles} ${_tmpFiles}) 
  endforeach()

  set(moList)
  foreach(_currentMoFile ${moFiles})
    set(_lang)
    if (${_currentMoFile} MATCHES "/locale/([^/\\\\]+)/LC_MESSAGES/${basename}.mo")
      set(_lang ${CMAKE_MATCH_1})
    elseif (${_currentMoFile} MATCHES "/([^/\\\\]+)/${basename}.mo")
      set(_lang ${CMAKE_MATCH_1})
    elseif (${_currentMoFile} MATCHES "/locale/([^./\\\\]+).mo")
      get_filename_component(_tmpPath ${_currentMoFile} PATH)
      if (EXISTS ${_tmpPath}/${basename}.po OR EXISTS ${_tmpPath}/${basename}.pot)
        set(_lang ${CMAKE_MATCH_1})
      endif()
    endif()
    if (_lang)
      GET_MO_DIR(_moDir _moInstDir ${_lang})
      SET(_moFile ${_moDir}/${basename}.mo)
      file(MAKE_DIRECTORY ${_moDir})

      ADD_CUSTOM_COMMAND(OUTPUT ${_moFile} COMMAND ${CMAKE_COMMAND} -E copy ${_currentMoFile} ${_moFile}  DEPENDS ${_currentMoFile})
      list(APPEND moList ${_moFile})

      install(FILES ${_moFile} DESTINATION ${_moInstDir} RENAME ${basename}.mo)
    endif()
  endforeach()

  add_custom_target(copy_po_dependencies_${basename} ALL DEPENDS ${moList})
endfunction()
