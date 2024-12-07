# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later
# (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

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
