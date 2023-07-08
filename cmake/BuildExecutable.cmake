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

function(BUILD_EXECUTABLE TARGET)
  if(NOT APPLE)
    target_link_options(${TARGET} PRIVATE "LINKER:--as-needed")
  endif()
  file(MAKE_DIRECTORY ${BINDIR})
  if (OBJECT_FIXUP_REQUIRED STREQUAL "ON")
    ADD_CUSTOM_COMMAND(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/$<TARGET_FILE_NAME:${TARGET}>" "${BINDIR}/$<TARGET_FILE_NAME:${TARGET}>")
  else()
    set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${BINDIR}")
  endif()
  install(TARGETS ${TARGET} RUNTIME DESTINATION "${BININSTDIR}")

  if(CV2PDB_EXE)
    add_custom_command(
      OUTPUT "${BINDIR}/${TARGET}.pdb"
      DEPENDS ${TARGET}
      COMMAND
	${CMAKE_COMMAND}
	  -E env "WINEPATH=Z:${VC_PATH}"
	  wine "${CV2PDB_EXE}"
	  "$<TARGET_FILE:${TARGET}>"
	  "${CMAKE_CURRENT_BINARY_DIR}/$<TARGET_FILE_PREFIX:${TARGET}>$<TARGET_FILE_BASE_NAME:${TARGET}>-tmp$<TARGET_FILE_SUFFIX:${TARGET}>"
	  "${BINDIR}/${TARGET}.pdb"
    )
    add_custom_target(${TARGET}.pdb ALL DEPENDS "${BINDIR}/${TARGET}.pdb")
    install(FILES "${BINDIR}/${TARGET}.pdb" DESTINATION "${BININSTDIR}")
  endif()
endfunction()
