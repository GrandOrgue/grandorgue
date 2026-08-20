# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later
# (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

function(BUILD_LIBRARY TARGET)
  file(MAKE_DIRECTORY ${LIBDIR})
  add_library(${TARGET} SHARED ${ARGN})
  set_target_properties(${TARGET} PROPERTIES VERSION ${NUM_VERSION})

  # When cv2pdb is going to patch the library (see below) the linked library must stay in the
  # build tree, still carrying its original DWARF debug info: cv2pdb needs to read that DWARF
  # info every time it runs, and it must never read back a copy that itself has already been
  # patched by a previous cv2pdb run. So this case reuses the same POST_BUILD-copy pattern as
  # OBJECT_FIXUP_REQUIRED, just for a different reason: keeping LIBDIR a derived copy rather
  # than the linker's actual output.
  if (OBJECT_FIXUP_REQUIRED STREQUAL "ON" OR (CV2PDB_EXE AND GO_SPLIT_DEBUG_SYMBOLS))
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

  if(UNIX AND NOT APPLE AND GO_SPLIT_DEBUG_SYMBOLS)
    install(CODE "
      # \$ENV{DESTDIR} is empty for TGZ (CMAKE_INSTALL_PREFIX is the staging dir),
      # but set to the staging root for RPM (CMAKE_INSTALL_PREFIX stays /usr).
      set(_lib \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${LIBINSTDIR}/lib${TARGET}.so.${NUM_VERSION}\")
      # Extract debug info into a separate .dbg file alongside the library.
      execute_process(COMMAND ${CMAKE_OBJCOPY} --only-keep-debug \"\${_lib}\" \"\${_lib}.dbg\")
      # objcopy needs to read the .dbg file to compute its CRC for the debuglink section.
      # It looks for the file by bare name in WORKING_DIRECTORY, so we set that to the
      # directory containing both the library and the .dbg file.
      get_filename_component(_dir \"\${_lib}\" DIRECTORY)
      execute_process(COMMAND ${CMAKE_OBJCOPY} --strip-debug
        --add-gnu-debuglink=lib${TARGET}.so.${NUM_VERSION}.dbg \"\${_lib}\"
        WORKING_DIRECTORY \"\${_dir}\")
    ")
  endif()

  if(CV2PDB_EXE AND GO_SPLIT_DEBUG_SYMBOLS)
    # cv2pdb reads the DWARF-carrying library (still in the build tree, see above) and writes
    # the actually shipped copy in LIBDIR with the DWARF info replaced by a CodeView record
    # (RSDS: pdb GUID + age + path). Without that record dbghelp cannot locate nor validate the
    # pdb, so this patched copy, and not a plain copy of the linked library, is what must end up
    # in LIBDIR.
    add_custom_command(
      OUTPUT "${LIBDIR}/lib${TARGET}.pdb"
      DEPENDS ${TARGET}
      COMMAND
	${CMAKE_COMMAND}
	  -E env "WINEPATH=Z:${VC_PATH}"
	  wine "${CV2PDB_EXE}"
	  "${CMAKE_CURRENT_BINARY_DIR}/$<TARGET_FILE_NAME:${TARGET}>"
	  "${LIBDIR}/$<TARGET_FILE_NAME:${TARGET}>"
	  "${LIBDIR}/lib${TARGET}.pdb"
    )
    add_custom_target(lib${TARGET}.pdb ALL DEPENDS "${LIBDIR}/lib${TARGET}.pdb")
    install(FILES "${LIBDIR}/lib${TARGET}.pdb" DESTINATION "${LIBINSTDIR}")
  endif()
endfunction()
