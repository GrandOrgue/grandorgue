# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later
# (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

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

  install(
    TARGETS ${TARGET} 
    RUNTIME DESTINATION ${LIBINSTDIR} LIBRARY DESTINATION ${LIBINSTDIR}
    NAMELINK_SKIP
    # these permissions required for building rmp on debian/ubuntu
    # otherwise Autoprov doesn't work
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
      GROUP_EXECUTE GROUP_READ
      WORLD_EXECUTE WORLD_READ
  )

  if(CV2PDB_EXE)
    add_custom_command(
      OUTPUT "${LIBDIR}/lib${TARGET}.pdb"
      DEPENDS ${TARGET}
      COMMAND
	${CMAKE_COMMAND}
	  -E env "WINEPATH=Z:${VC_PATH}"
	  wine "${CV2PDB_EXE}"
	  "$<TARGET_FILE:${TARGET}>"
	  "${CMAKE_CURRENT_BINARY_DIR}/$<TARGET_FILE_PREFIX:${TARGET}>$<TARGET_FILE_BASE_NAME:${TARGET}>-tmp$<TARGET_FILE_SUFFIX:${TARGET}>"
	  "${LIBDIR}/lib${TARGET}.pdb"
    )
    add_custom_target(lib${TARGET}.pdb ALL DEPENDS "${LIBDIR}/lib${TARGET}.pdb")
    install(FILES "${LIBDIR}/lib${TARGET}.pdb" DESTINATION "${LIBINSTDIR}")
  endif()
endfunction()
