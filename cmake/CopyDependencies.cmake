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

include(${CMAKE_SOURCE_DIR}/cmake/ExtractLibraryDirs.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/WriteStatus.cmake)

function(CopyDependencies instpath)
  set(statusfile "${CMAKE_BINARY_DIR}/cmake.status")
  WriteStatus("${statusfile}")

  ExtractLibraryDirs(searchdirs ${ARGN})

  set(targetfile "${CMAKE_BINARY_DIR}/${instpath}")
  set(stampfile "${CMAKE_CURRENT_BINARY_DIR}/copy_dlls.stamp")

  add_custom_command(OUTPUT "${stampfile}"
    COMMAND ${CMAKE_COMMAND} -Dstatusfile="${statusfile}" -Dstampfile="${stampfile}" -Dtarget="${targetfile}" -Dsearchdirs="${searchdirs}" -P "${CMAKE_SOURCE_DIR}/cmake/FixupBundle.cmake"
    DEPENDS ${targetfile} "${CMAKE_SOURCE_DIR}/cmake/FixupBundle.cmake" "${statusfile}")
  add_custom_target(copy_dlls_dependencies ALL DEPENDS "${stampfile}")

  install(CODE "set(searchdirs \"${searchdirs}\")")
  install(CODE "set(statusfile \"${statusfile}\")")
  install(CODE "set(target \"\$ENV{DESTDIR}/\${CMAKE_INSTALL_PREFIX}/${instpath}\")")
  install(SCRIPT "${CMAKE_SOURCE_DIR}/cmake/FixupBundle.cmake")
endfunction()