# GrandOrgue - free pipe organ simulator
# 
# Copyright 2006 Milan Digital Audio LLC
# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later
# (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

include(${CMAKE_SOURCE_DIR}/cmake/ExtractLibraryDirs.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/WriteStatus.cmake)

function(CopyDependencies app instpath)
  set(statusfile "${CMAKE_BINARY_DIR}/cmake.status")
  WriteStatus("${statusfile}")

  ExtractLibraryDirs(searchdirs ${ARGN})

  set(targetfile "${CMAKE_BINARY_DIR}/${instpath}")
  string(REPLACE ";" "$<SEMICOLON>" cmd_searchdirs "${searchdirs}")

  add_custom_command(TARGET ${app} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -Dstatusfile="${statusfile}" -Dbundledtarget="${targetfile}" -Dsearchdirs="${cmd_searchdirs}" -P "${CMAKE_SOURCE_DIR}/cmake/FixupBundle.cmake"
    DEPENDS ${targetfile} "${CMAKE_SOURCE_DIR}/cmake/FixupBundle.cmake" "${statusfile}")

  install(CODE "execute_process(COMMAND \"${CMAKE_COMMAND}\" \"-Dstatusfile=${statusfile}\" \"-Dbundledtarget=\$ENV{DESTDIR}/\${CMAKE_INSTALL_PREFIX}/${instpath}\" \"-Dsearchdirs=${searchdirs}\" -P \"${CMAKE_SOURCE_DIR}/cmake/FixupBundle.cmake\")")
endfunction()
