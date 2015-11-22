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

cmake_minimum_required(VERSION 2.8)

project(GrandOrgue CXX C)

# Build configuration options
option(UNICODE                "Build a Unicode configuration" ON)
option(RTAUDIO_USE_CORE       "Enable RtAudio support for Core Audio (Rt and PortAudio - OS X only)" ON)
option(RTAUDIO_USE_JACK       "Enable RtAudio support for Jack (Rt and PortAudio)" ON)
option(RTAUDIO_USE_OSS        "Enable RtAudio support for OSS (Rt and PortAudio - Linux only)" OFF)
option(RTAUDIO_USE_ALSA       "Enable RtAudio support for ALSA (Rt and PortAudio - Linux only)" ON)
option(RTAUDIO_USE_DSOUND     "Enable RtAudio support for DirectSound (Rt and PortAudio - Windows only)" ON)
option(RTAUDIO_USE_ASIO       "Enable RtAudio support for ASIO (Rt and PortAudio - Windows only)" ON)
option(RTAUDIO_USE_WMME       "Enable RtAudio support for WMME (PortAudio only - Windows only)" ON)
option(RTAUDIO_USE_WDMKS      "Enable RtAudio support for WDMKS (PortAudio only - Windows only)" ON)
option(RTAUDIO_USE_WASAPI     "Enable RtAudio support for WASAPI (PortAudio only - Windows only)" ON)
option(RTMIDI_USE_CORE        "Enable RtMidi support for Core Audio (OS X only)" ON)
option(RTMIDI_USE_JACK        "Enable RtMidi support for Jack" ON)
option(RTMIDI_USE_ALSA        "Enable RtMidi support for ALSA (Linux only)" ON)
option(RTMIDI_USE_MM          "Enable RtMidi support for MM (Windows only)" ON)
option(INSTALL_DEMO           "Install demo sampleset" ON)
option(USE_INTERNAL_PORTAUDIO "Use builtin PortAudio sources" ON)
option(USE_INTERNAL_FFTW      "Use builtin fftw3 sources" ON)
option(USE_INTERNAL_WAVPACK   "Use builtin WavPack sources" ON)
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
set(NUM_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_STAGE}.${VERSION_REVISION})
string(REPLACE "." "," NUM_WIN_VERSION ${NUM_VERSION})

if (CMAKE_BUILD_TYPE STREQUAL "")
   message(STATUS "No build configuration specified. Defaulting to 'Release' build.")
   set(CMAKE_BUILD_TYPE "Release")
endif ()

# only use options supported by the compiler
include(CheckIncludeFileCXX)
include(${CMAKE_SOURCE_DIR}/cmake/AddOption.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/AddCXXOption.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/FindTools.cmake)

add_cxx_option(-std=c++0x)
CHECK_INCLUDE_FILE_CXX(cstdatomic HAVE_CSTDATOMIC)
CHECK_INCLUDE_FILE_CXX(atomic HAVE_ATOMIC)
CHECK_INCLUDE_FILE_CXX(mutex HAVE_MUTEX)

if(UNIX AND NOT APPLE)
  install(FILES GrandOrgue.desktop DESTINATION share/applications)
  install(FILES GrandOrgue.png DESTINATION share/pixmaps)
  install(FILES grandorgue.xml DESTINATION share/mime/packages)
  install(FILES GrandOrgue.1 DESTINATION share/man/man1)
  install(FILES GrandOrgue.appdata.xml DESTINATION share/appdata)
endif()

IF (APPLE)
  SET(RESOURCEDIR "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.app/Contents/Resources")
  SET(RESOURCEINSTDIR "${PROJECT_NAME}.app/Contents/Resources")
ELSE()
  SET(RESOURCEDIR "${CMAKE_BINARY_DIR}/share/${PROJECT_NAME}")
  SET(RESOURCEINSTDIR "share/${PROJECT_NAME}")
ENDIF()

# setup compiler flags for debug vs release compiles
add_option(-ffast-math)
add_option(-Wall)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
   add_option(-g)
   add_option(-O0)
   add_option(-mmmx)
   add_option(-msse)
   add_option(-msse2)
   add_option(-msse3)
   add_option(-funit-at-a-time)
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
   add_option(-fomit-frame-pointer)
   add_option(-O3)
   add_option(-mmmx)
   add_option(-msse)
   add_option(-msse2)
   add_option(-msse3)
   add_option(-funroll-loops)
   add_definitions(-DNDEBUG)
endif ()

add_option(-mstackrealign)

if (APPLE)
   # see the following URL for information about these variables
   # https://developer.apple.com/library/mac/#documentation/General/Reference/InfoPlistKeyReference/Articles/CoreFoundationKeys.html
   set(BUNDLE_CFBundleShortVersionString ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_STAGE})
   set(BUNDLE_CFBundleVersion ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_STAGE}.${VERSION_REVISION})

   set(CPACK_PACKAGE_ICON "${RESOURCEDIR}/GrandOrgue.icns")

   configure_file(${CMAKE_SOURCE_DIR}/src/grandorgue/resource/Info.plist.in ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.app/Contents/Info.plist)
   INSTALL(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.app/Contents/Info.plist DESTINATION "${PROJECT_NAME}.app/Contents/")

   ADD_CUSTOM_COMMAND(OUTPUT "${RESOURCEDIR}/GrandOrgue.icns" COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/src/grandorgue/resource/GrandOrgue.icns "${RESOURCEDIR}/GrandOrgue.icns" DEPENDS ${CMAKE_SOURCE_DIR}/src/grandorgue/resource/GrandOrgue.icns)
   INSTALL(FILES "${RESOURCEDIR}/GrandOrgue.icns" DESTINATION "${RESOURCEINSTDIR}")
   ADD_CUSTOM_TARGET(copy_bundle ALL DEPENDS "${RESOURCEDIR}/GrandOrgue.icns")

   set(CPACK_BINARY_DRAGNDROP ON)
endif ()

# include RtAudio
add_subdirectory(src/rt lib/rt)
include_directories(src/rt/include)

# include portaudio
if (USE_INTERNAL_PORTAUDIO)
  add_subdirectory(src/portaudio lib/portaudio)
  include_directories(src/portaudio/include)
  set(PORTAUDIO_LIBRARIES PortAudio)
else()
  if(DEFINED PORTAUDIO_INCLUDE_DIRS)
    include_directories(${PORTAUDIO_INCLUDE_DIRS})
  endif()
  if(NOT DEFINED PORTAUDIO_LIBRARIES)
    set(PORTAUDIO_LIBRARIES portaudio)
  endif()
endif()

# include FFTW
if (USE_INTERNAL_FFTW)
  add_subdirectory(src/fftw lib/fftw)
  include_directories(${CMAKE_BINARY_DIR}/lib/fftw/)
  set(FFTW_LIBRARIES fftw)
else()
  if(DEFINED FFTW_INCLUDE_DIRS)
    include_directories(${FFTW_INCLUDE_DIRS})
  endif()
  if(NOT DEFINED FFTW_LIBRARIES)
    set(FFTW_LIBRARIES fftw3f)
  endif()
endif()

# include WAVPACK
if (USE_INTERNAL_WAVPACK)
  add_subdirectory(src/wavpack lib/wavpack)
  include_directories(src/wavpack/include)
  set(WAVPACK_LIBRARIES WavPack)
else()
  if(DEFINED WAVPACK_INCLUDE_DIRS)
    include_directories(${WAVPACK_INCLUDE_DIRS})
  endif()
  if(NOT DEFINED WAVPACK_LIBRARIES)
    set(WAVPACK_LIBRARIES wavpack)
  endif()
endif()

add_subdirectory(src/build)
add_subdirectory(src/grandorgue bin)
add_subdirectory(po)
add_subdirectory(help)
add_subdirectory(sounds)
if (INSTALL_DEMO)
  add_subdirectory(demo)
endif()
add_subdirectory(packages)

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
