# Script for macOS code signing

if(NOT DEFINED APP_DIR)
  set(APP_DIR "${CPACK_TEMPORARY_INSTALL_DIRECTORY}/ALL_IN_ONE")
endif()

file(GLOB LIBSTOSIGN "${APP_DIR}/GrandOrgue.app/Contents/Frameworks/*.*")
foreach(LIBTOSIGN ${LIBSTOSIGN})
  execute_process(COMMAND codesign --force --sign - "${LIBTOSIGN}")
endforeach()
execute_process(COMMAND codesign --force --sign - "${APP_DIR}/GrandOrgue.app/Contents/MacOS/GrandOrguePerfTest")
execute_process(COMMAND codesign --force --sign - "${APP_DIR}/GrandOrgue.app/Contents/MacOS/GrandOrgueTool")
execute_process(COMMAND codesign --force --sign - "${APP_DIR}/GrandOrgue.app")
message("Checking code signature...")
execute_process(COMMAND codesign --verify --deep "${APP_DIR}/GrandOrgue.app")
