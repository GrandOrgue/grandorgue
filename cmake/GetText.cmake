# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later
# (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

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

