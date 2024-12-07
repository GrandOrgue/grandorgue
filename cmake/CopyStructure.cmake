# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later
# (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

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
