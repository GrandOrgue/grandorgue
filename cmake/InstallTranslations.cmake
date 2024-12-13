# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later
# (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

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
