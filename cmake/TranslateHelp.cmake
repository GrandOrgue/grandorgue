# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later
# (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

function(PO4A_BUILD_HELP xmlfile)
  FOREACH(_currentPoFile ${ARGN})
    GET_FILENAME_COMPONENT(_lang ${_currentPoFile} NAME_WE)
    ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_lang}.xml COMMAND ${PO4A_TRANSLATE}
      ARGS -f docbook -m ${xmlfile} -p ${_currentPoFile} -l ${CMAKE_CURRENT_BINARY_DIR}/${_lang}.xml -k 0
      DEPENDS ${_currentPoFile} ${xmlfile})
          BUILD_HELPFILE(${CMAKE_CURRENT_BINARY_DIR}/${_lang}.xml _${_lang})
  ENDFOREACH()
endfunction()

function(PO4A_UPDATE_POT target _potFile xmlfile)
  ADD_CUSTOM_TARGET(${target} COMMAND ${PO4A_GETTEXT} ARGS -f docbook -m ${xmlfile} -p ${CMAKE_CURRENT_BINARY_DIR}/${_potFile} --package-name=GrandOrgue --msgid-bugs-address="ourorgan-developers@lists.sourceforge.net" WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endfunction()

