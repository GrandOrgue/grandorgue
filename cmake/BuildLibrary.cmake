# GrandOrgue - free pipe organ simulator
# 
# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2018 GrandOrgue contributors (see AUTHORS)
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

function(BUILD_LIBRARY TARGET)
  file(MAKE_DIRECTORY ${LIBDIR})
  add_library(${TARGET} SHARED ${ARGN})
  set_target_properties(${TARGET} PROPERTIES VERSION ${NUM_VERSION})

  if (OBJECT_FIXUP_REQUIRED STREQUAL "ON")
    ADD_CUSTOM_COMMAND(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/$<TARGET_FILE_NAME:${TARGET}>" "${LIBDIR}/$<TARGET_FILE_NAME:${TARGET}>")
  else()
    set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${LIBDIR})
    set_target_properties(${TARGET} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${LIBDIR})
  endif()

  install(TARGETS ${TARGET} RUNTIME DESTINATION ${LIBINSTDIR} LIBRARY DESTINATION ${LIBINSTDIR} NAMELINK_SKIP)
endfunction()
