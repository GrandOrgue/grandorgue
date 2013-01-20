cmake_minimum_required(VERSION 2.6)

project(GrandOrgue CXX C)

# Build configuration options
option(UNICODE                "Build a Unicode configuration" OFF)
option(RTAUDIO_USE_CORE       "Enable RtAudio support for Core Audio (Rt and PortAudio - OS X only)" ON)
option(RTAUDIO_USE_JACK       "Enable RtAudio support for Jack (Rt and PortAudio - Linux and OS X only)" ON)
option(RTAUDIO_USE_OSS        "Enable RtAudio support for OSS (Rt and PortAudio - Linux only)" OFF)
option(RTAUDIO_USE_ALSA       "Enable RtAudio support for ALSA (Rt and PortAudio - Linux only)" ON)
option(RTAUDIO_USE_DSOUND     "Enable RtAudio support for DirectSound (Rt and PortAudio - Windows only)" ON)
option(RTAUDIO_USE_ASIO       "Enable RtAudio support for ASIO (Rt and PortAudio - Windows only)" ON)
option(RTAUDIO_USE_WMME       "Enable RtAudio support for WMME (PortAudio only - Windows only)" ON)
option(RTAUDIO_USE_WDMKS      "Enable RtAudio support for WDMKS (PortAudio only - Windows only)" ON)
option(RTAUDIO_USE_WASAPI     "Enable RtAudio support for WASAPI (PortAudio only - Windows only)" ON)
option(RTMIDI_USE_CORE        "Enable RtMidi support for Core Audio (OS X only)" ON)
option(RTMIDI_USE_JACK        "Enable RtMidi support for Jack (Linux only)" ON)
option(RTMIDI_USE_ALSA        "Enable RtMidi support for ALSA (Linux only)" ON)
option(RTMIDI_USE_MM          "Enable RtMidi support for MM (Windows only)" ON)
option(USE_INTERNAL_PORTAUDIO "Use builtin PortAudio sources" ON)
option(USE_INTERNAL_FFTW      "Use builtin fftw3 sources" ON)
if (WIN32 OR APPLE)
   option(INSTALL_DEPEND      "Copy dependencies (wxWidgets libraries and Translations) on installation" ON)
else ()
   option(INSTALL_DEPEND      "Copy dependencies (wxWidgets libraries and Translations) on installation" OFF)
endif ()

# Project version numbers
set(VERSION_MAJOR 0)
set(VERSION_MINOR 3)
set(VERSION_STAGE 1)
if (NOT VERSION_REVISION)
  set(VERSION_REVISION 0)
endif()
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_STAGE}.${VERSION_REVISION})

if (CMAKE_BUILD_TYPE STREQUAL "")
   message(STATUS "No build configuration specified. Defaulting to 'Release' build.")
   set(CMAKE_BUILD_TYPE "Release")
endif ()

# only use options supported by the compiler
include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)
include(CheckIncludeFileCXX)
macro(add_option _option)
  set(_name "OPTION_${_option}")
  CHECK_CXX_COMPILER_FLAG("-${_option}" ${_name})
  if(${_name})
   add_definitions("-${_option}")
  endif()
endmacro(add_option option)

macro(add_cxx_option _option)
  set(_name "OPTION_LINK_${_option}")
  string(REGEX REPLACE "=" "-EQ" _name ${_name})
  string(REGEX REPLACE "\\+" "-P" _name ${_name})
  SET(CMAKE_REQUIRED_FLAGS "${_option}")
  CHECK_CXX_COMPILER_FLAG("" ${_name})
  if(${_name})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${_option}")
  endif()
endmacro()

add_cxx_option(-std=c++0x)
CHECK_INCLUDE_FILE_CXX(cstdatomic HAVE_CSTDATOMIC)
CHECK_INCLUDE_FILE_CXX(atomic HAVE_ATOMIC)
CHECK_INCLUDE_FILE_CXX(mutex HAVE_MUTEX)

if(UNIX AND NOT APPLE)
  install(FILES GrandOrgue.desktop DESTINATION share/applications)
  install(FILES GrandOrgue.png DESTINATION share/pixmaps)
  install(FILES grandorgue.xml DESTINATION share/mime/packages)
  install(FILES GrandOrgue.1 DESTINATION share/man/man1)
endif()

# setup compiler flags for debug vs release compiles
add_option(ffast-math)
add_option(Wall)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
   add_option(g)
   add_option(O0)
   add_option(mmmx)
   add_option(msse)
   add_option(msse2)
   add_option(msse3)
   add_option(funit-at-a-time)
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
   add_option(fomit-frame-pointer)
   add_option(O3)
   add_option(mmmx)
   add_option(msse)
   add_option(msse2)
   add_option(msse3)
   add_option(funroll-loops)
   add_option(DNDEBUG)
endif ()

add_option(mstackrealign)

if (APPLE)
   # see the following URL for information about these variables
   # https://developer.apple.com/library/mac/#documentation/General/Reference/InfoPlistKeyReference/Articles/CoreFoundationKeys.html
   set(BUNDLE_CFBundleShortVersionString ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_STAGE})
   set(BUNDLE_CFBundleVersion ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_STAGE}.${VERSION_REVISION})
   if (CMAKE_BUILD_TYPE STREQUAL "Debug")
      set(BUNDLE_CFBundleDisplayName "${PROJECT_NAME} ${BUNDLE_CFBundleVersion} Debug")
   else ()
      set(BUNDLE_CFBundleDisplayName "${PROJECT_NAME} ${BUNDLE_CFBundleVersion}")
   endif ()

   set(CPACK_BUNDLE_PLIST ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.app/Contents/Info.plist)
   set(CPACK_BUNDLE_NAME ${PROJECT_NAME})
   set(CPACK_BUNDLE_ICON ${CMAKE_CURRENT_SOURCE_DIR}/src/grandorgue/resource/GrandOrgue.icns)
   set(CPACK_PACKAGE_ICON ${CMAKE_CURRENT_SOURCE_DIR}/src/grandorgue/resource/GrandOrgue.icns)
endif ()

# include RtAudio
add_subdirectory(src/rt lib/rt)
# include portaudio
if (USE_INTERNAL_PORTAUDIO)
  add_subdirectory(src/portaudio lib/portaudio)
endif()
if (USE_INTERNAL_FFTW)
  add_subdirectory(src/fftw lib/fftw)
endif()
add_subdirectory(src/build)
add_subdirectory(src/grandorgue bin)
add_subdirectory(po)
add_subdirectory(help)

set(CPACK_PACKAGE_NAME "GrandOrgue")
set(CPACK_PACKAGE_VENDOR "Our Organ")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "GrandOrgue - OpenSource Virtual Pipe Organ Software")
set(CPACK_PACKAGE_VERSION ${VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_STAGE}.${VERSION_REVISION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "GrandOrgue")
set(CPACK_RPM_PACKAGE_LICENSE "GPL v2+")
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/license.txt)
set(CPACK_PACKAGE_CONTACT "ourorgan-developers@lists.sourceforge.net")
set(CPACK_STRIP_FILES true)
set(CPACK_PACKAGE_EXECUTABLES "GrandOrgue" "GrandOrgue")
SET(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
  WriteRegStr HKCR \\\".organ\\\" \\\"\\\" \\\"GrandOrgue.odf\\\"
  WriteRegStr HKCR \\\"GrandOrgue.odf\\\" \\\"\\\" \\\"GrandOrgue organ definition file\\\"
  WriteRegStr HKCR \\\"GrandOrgue.odf\\\\DefaultIcon\\\" \\\"\\\" \\\"$INSTDIR\\\\bin\\\\GrandOrgue.exe,0\\\"
  WriteRegStr HKCR \\\"GrandOrgue.odf\\\\shell\\\" \\\"\\\" \\\"open\\\"
  WriteRegStr HKCR \\\"GrandOrgue.odf\\\\shell\\\\open\\\\command\\\" \\\"\\\" '$INSTDIR\\\\bin\\\\GrandOrgue.exe \\\"%1\\\"'
")
SET(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
  ReadRegStr $R0 HKCR \\\".organ\\\" \\\"\\\"
  StrCmp $R0 \\\"GrandOrgue.odf\\\" 0 +2
    DeleteRegKey HKCR \\\".organ\\\"

  DeleteRegKey HKCR \\\"GrandOrgue.odf\\\"
")
include(CPack)

message(STATUS "  Project                     : ${PROJECT_NAME}")
message(STATUS "  Description                 : ${CPACK_PACKAGE_DESCRIPTION_SUMMARY}")
message(STATUS "  Version                     : ${VERSION}")
message(STATUS "============================================================================")
message(STATUS " ")
