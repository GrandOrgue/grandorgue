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
