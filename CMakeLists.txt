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

cmake_minimum_required(VERSION 3.10)

# Project version numbers
if(NOT DEFINED VERSION)
  file(STRINGS "version.txt" VERSION)
  if(NOT VERSION)
    message(FATAL_ERROR "Unable to get version from version.txt")
  endif()
endif()

project(
  GrandOrgue
  VERSION "${VERSION}"
  DESCRIPTION "GrandOrgue - free pipe organ simulator"
  LANGUAGES CXX C
)
set(NUM_VERSION "${PROJECT_VERSION}")
string(REPLACE "." "," NUM_WIN_VERSION ${NUM_VERSION})

if(NOT DEFINED BUILD_VERSION)
  set(BUILD_VERSION "0.local")
elseif(NOT BUILD_VERSION)
  set(BUILD_VERSION "0.0")
endif()

set(FULL_VERSION "${PROJECT_VERSION}-${BUILD_VERSION}")

# Build configuration options
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
option(GO_USE_JACK	      "Use native Jack output" ON)
if (WIN32 OR APPLE)
   option(INSTALL_DEPEND      "Copy dependencies (wxWidgets libraries and Translations) on installation" ON)
else ()
   option(INSTALL_DEPEND      "Copy dependencies (wxWidgets libraries and Translations) on installation" OFF)
endif ()
option(GO_SEPARATE_LINUX_PACKAGES "Generate separate linux packages for resources and demo" OFF)

# only use options supported by the compiler
include(CheckIncludeFileCXX)
include(CheckFunctionExists)
include(FindPkgConfig)
include(${CMAKE_SOURCE_DIR}/cmake/AddOption.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/AddCXXOption.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/FindTools.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/BuildLibrary.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/BuildExecutable.cmake)

add_cxx_option(-std=c++11)
CHECK_INCLUDE_FILE_CXX(atomic HAVE_ATOMIC)
CHECK_INCLUDE_FILE_CXX(mutex HAVE_MUTEX)

IF (APPLE)
  set(BININSTDIR "${PROJECT_NAME}.app/Contents/MacOS")
  SET(LIBINSTDIR "${PROJECT_NAME}.app/Contents/MacOS")
  SET(RESOURCEINSTDIR "${PROJECT_NAME}.app/Contents/Resources")
  SET(RPATH_PREFIX "@executable_path")
ELSE()
  set(BININSTDIR "bin")
  IF (WIN32)
    SET(LIBINSTDIR "bin")
  ELSE ()
    SET(LIBINSTDIR "lib" CACHE STRING "library directory")
  ENDIF ()
  SET(RESOURCEINSTDIR "share/${PROJECT_NAME}")
  SET(RPATH_PREFIX "$ORIGIN")
ENDIF()
SET(BINDIR "${CMAKE_BINARY_DIR}/${BININSTDIR}")
SET(LIBDIR "${CMAKE_BINARY_DIR}/${LIBINSTDIR}")
SET(RESOURCEDIR "${CMAKE_BINARY_DIR}/${RESOURCEINSTDIR}")

if (APPLE AND INSTALL_DEPEND STREQUAL "ON")
  set(OBJECT_FIXUP_REQUIRED "ON")
else()
  set(OBJECT_FIXUP_REQUIRED "OFF")
endif()

file(RELATIVE_PATH RPATH "${BINDIR}" "${LIBDIR}")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
SET(CMAKE_INSTALL_RPATH "${RPATH_PREFIX}/${RPATH}")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)

# setup compiler flags for debug vs release compiles
add_option(-Wall)
add_definitions(-DGO_STD_MUTEX)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_option(-g3)
else()
  add_option(-O3)
  add_option(-g)
  add_definitions(-DNDEBUG)
  add_option(-fomit-frame-pointer)
  add_option(-funroll-loops)
  add_option(-ffast-math)
endif ()

if (RTAUDIO_USE_JACK OR RTMIDI_USE_JACK OR GO_USE_JACK)
  pkg_check_modules(JACK REQUIRED IMPORTED_TARGET jack)
endif()

# Add ASIO variable
if(WIN32 AND RTAUDIO_USE_ASIO)
  add_definitions(-DASIO_INCLUDED)
endif()

if(RTAUDIO_USE_ASIO AND NOT ASIO_SDK_DIR)
  set(ASIO_SDK_DIR "${CMAKE_SOURCE_DIR}/ext/rt/asio")
endif()

# include RtAudio
if(NOT RTAUDIO_SRC_DIR)
  set(RTAUDIO_SRC_DIR "${CMAKE_SOURCE_DIR}/submodules/RtAudio")
  if(NOT EXISTS "${RTAUDIO_SRC_DIR}/RtAudio.h")
    message(
      FATAL_ERROR 
      "${RTAUDIO_SRC_DIR}/RtAudio.h file does not exist."
      "Possible the RtAudio submodule has not been updated."
      "Try to execute 'git submodule update --init --recursive' in the source directory"
    )
  endif()
endif()
if(NOT RTMIDI_SRC_DIR)
  set(RTMIDI_SRC_DIR "${CMAKE_SOURCE_DIR}/submodules/RtMidi")
  if(NOT EXISTS "${RTMIDI_SRC_DIR}/RtMidi.h")
    message(
      FATAL_ERROR 
      "${RTMIDI_SRC_DIR}/RtMidi.h file does not exist."
      "Possible the RtMidi submodule has not been updated."
      "Try to execute 'git submodule update --init --recursive' in the source directory"
    )
  endif()
endif()
add_subdirectory(src/rt)
set(RT_LIBRARIES RtAudio RtMidi)
set(RT_INCLUDE_DIRS ${RTMIDI_SRC_DIR} ${RTAUDIO_SRC_DIR})

# include portaudio
if (USE_INTERNAL_PORTAUDIO)
  set(PORTAUDIO_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/submodules/PortAudio/include)
  add_subdirectory(src/portaudio)
  set(PORTAUDIO_LIBRARIES PortAudio)
else()
  pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)
endif()

# include FFTW
pkg_check_modules(FFTW REQUIRED fftw3f)

pkg_check_modules(ZLIB REQUIRED zlib)

pkg_check_modules(WAVPACK REQUIRED wavpack)

find_package(wxWidgets REQUIRED html net adv core base)
message(STATUS "  wxWidgets Unicode?          : ${wxWidgets_USE_UNICODE}")
message(STATUS "  wxWidgets Debug?            : ${wxWidgets_USE_DEBUG}")
message(STATUS "  wxWidgets Static linking    : ${wxWidgets_USE_STATIC}")
message(STATUS "  wxWidgets version           : ${wxWidgets_VERSION}")
message(STATUS "  wxWidgets config binary     : ${wxWidgets_CONFIG_EXECUTABLE}")
message(STATUS "  wxWidgets configuration     : ${wxWidgets_CONFIGURATION}")
message(STATUS "============================================================================")

add_subdirectory(src/build)
add_subdirectory(src/images)
add_subdirectory(src/core)
add_subdirectory(src/grandorgue)
add_subdirectory(src/tools)
add_subdirectory(po)
add_subdirectory(help)
add_subdirectory(sounds)
add_subdirectory(packages)
add_subdirectory(resources)


# packaging

set(CPACK_PACKAGE_NAME "grandorgue")
set(CPACK_PACKAGE_VENDOR "Our Organ")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "GrandOrgue - OpenSource Virtual Pipe Organ Software")
set(
  CPACK_PACKAGE_DESCRIPTION
  "GrandOrgue is a virtual pipe organ sample player application supporting a HW1 compatible file format"
)
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_RELEASE ${BUILD_VERSION})

set(CPACK_PACKAGE_INSTALL_DIRECTORY "GrandOrgue")
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE)
set(CPACK_PACKAGE_CONTACT "osamarin68@gmail.com")
set(CPACK_PACKAGE_EXECUTABLES "GrandOrgue" "GrandOrgue")

set(CPACK_SOURCE_IGNORE_FILES "/\\\\.git/" "/build/")

# components
set(CPACK_COMPONENTS_ALL Unspecified resources demo)
set(CPACK_COMPONENT_UNSPECIFIED_DISPLAY_NAME "GrandOrgue")

set(CPACK_COMPONENT_RESOURCES_DISPLAY_NAME "GrandOrgue Resource Files")
set(
  CPACK_COMPONENT_RESOURCES_DESCRIPTION
  "This package contains the various resource files for GrandOrgue"
)

set(CPACK_COMPONENT_DEMO_DISPLAY_NAME "GrandOrgue Demo Sampleset")
set(
  CPACK_COMPONENT_DEMO_DESCRIPTION 
  "This package contains the demo sampleset for GrandOrgue"
)

set(CPACK_COMPONENT_UNSPECIFIED_HIDDEN OFF)
set(CPACK_COMPONENT_UNSPECIFIED_DEPENDS resources)
set(CPACK_COMPONENT_DEMO_DEPENDS Unspecified)

if (APPLE)
   # see the following URL for information about these variables
   # https://developer.apple.com/library/mac/#documentation/General/Reference/InfoPlistKeyReference/Articles/CoreFoundationKeys.html
   set(BUNDLE_CFBundleShortVersionString ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_STAGE})
   set(BUNDLE_CFBundleVersion ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_STAGE})

   set(CPACK_PACKAGE_ICON "${RESOURCEDIR}/GrandOrgue.icns")

   configure_file(${CMAKE_SOURCE_DIR}/src/grandorgue/resource/Info.plist.in ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.app/Contents/Info.plist)
   INSTALL(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.app/Contents/Info.plist DESTINATION "${PROJECT_NAME}.app/Contents/")

  set (CPACK_SYSTEM_NAME "osx")
  set (CPACK_GENERATOR DragNDrop)

elseif (WIN32)

  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CPACK_STRIP_FILES OFF)
  else()
    set(CPACK_STRIP_FILES ON)
  endif()

  set (CPACK_SYSTEM_NAME "windows")
  set (CPACK_GENERATOR ZIP NSIS)
  SET(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
    WriteRegStr HKCR \\\".organ\\\" \\\"\\\" \\\"GrandOrgue.odf\\\"
    WriteRegStr HKCR \\\".orgue\\\" \\\"\\\" \\\"GrandOrgue.package\\\"
    WriteRegStr HKCR \\\"GrandOrgue.odf\\\" \\\"\\\" \\\"GrandOrgue organ definition file\\\"
    WriteRegStr HKCR \\\"GrandOrgue.odf\\\\DefaultIcon\\\" \\\"\\\" \\\"$INSTDIR\\\\bin\\\\GrandOrgue.exe,0\\\"
    WriteRegStr HKCR \\\"GrandOrgue.odf\\\\shell\\\" \\\"\\\" \\\"open\\\"
    WriteRegStr HKCR \\\"GrandOrgue.odf\\\\shell\\\\open\\\\command\\\" \\\"\\\" '$INSTDIR\\\\bin\\\\GrandOrgue.exe \\\"%1\\\"'
    WriteRegStr HKCR \\\"GrandOrgue.package\\\" \\\"\\\" \\\"GrandOrgue organ package\\\"
    WriteRegStr HKCR \\\"GrandOrgue.package\\\\DefaultIcon\\\" \\\"\\\" \\\"$INSTDIR\\\\bin\\\\GrandOrgue.exe,0\\\"
    WriteRegStr HKCR \\\"GrandOrgue.package\\\\shell\\\" \\\"\\\" \\\"open\\\"
    WriteRegStr HKCR \\\"GrandOrgue.package\\\\shell\\\\open\\\\command\\\" \\\"\\\" '$INSTDIR\\\\bin\\\\GrandOrgue.exe \\\"%1\\\"'
  ")
  SET(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
    ReadRegStr $R0 HKCR \\\".organ\\\" \\\"\\\"
    StrCmp $R0 \\\"GrandOrgue.odf\\\" 0 +2
      DeleteRegKey HKCR \\\".organ\\\"
    ReadRegStr $R0 HKCR \\\".orgue\\\" \\\"\\\"
    StrCmp $R0 \\\"GrandOrgue.package\\\" 0 +2
      DeleteRegKey HKCR \\\".orgue\\\"

    DeleteRegKey HKCR \\\"GrandOrgue.odf\\\"
    DeleteRegKey HKCR \\\"GrandOrgue.package\\\"
  ")

elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CPACK_STRIP_FILES OFF)
  else()
    set(CPACK_STRIP_FILES ON)
  endif()

  set(CPACK_SYSTEM_NAME "linux")
  set(CPACK_GENERATOR TGZ RPM DEB)

  set(CPACK_RPM_COMPONENT_INSTALL ${GO_SEPARATE_LINUX_PACKAGES})
  set(CPACK_RPM_PACKAGE_LICENSE "GPL v2+")
  set(CPACK_RPM_PACKAGE_RELEASE "${CPACK_PACKAGE_RELEASE}")
  set(CPACK_RPM_PACKAGE_GROUP "Productivity/Multimedia/Sound/Midi")
  set(CPACK_RPM_MAIN_COMPONENT Unspecified)
  set(CPACK_RPM_FILE_NAME RPM-DEFAULT)
  set(CPACK_RPM_PACKAGE_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION}")
  set(CPACK_RPM_UNSPECIFIED_PACKAGE_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION}")
  set(CPACK_RPM_UNSPECIFIED_PACKAGE_REQUIRES_POST "shared-mime-info, desktop-file-utils")
  set(CPACK_RPM_UNSPECIFIED_PACKAGE_REQUIRES_POSTUN "shared-mime-info, desktop-file-utils")
  set(CPACK_RPM_UNSPECIFIED_PACKAGE_REQUIRES "grandorgue-resources")
  set(CPACK_RPM_UNSPECIFIED_PACKAGE_SUGGESTS "grandorgue-demo")
  set(CPACK_RPM_RESOURCES_PACKAGE_SUMMARY "${CPACK_COMPONENT_RESOURCES_DISPLAY_NAME}")
  set(CPACK_RPM_RESOURCES_PACKAGE_ARCHITECTURE noarch)
  set(CPACK_RPM_RESOURCES_FILE_NAME RPM-DEFAULT)
  set(CPACK_RPM_DEMO_PACKAGE_SUMMARY "${CPACK_COMPONENT_DEMO_DISPLAY_NAME}")
  set(CPACK_RPM_DEMO_PACKAGE_ARCHITECTURE noarch)
  set(CPACK_RPM_DEMO_FILE_NAME RPM-DEFAULT)
  if(NOT GO_SEPARATE_LINUX_PACKAGES)
    # For automatic removal of previously installed suppackages
    set(CPACK_RPM_PACKAGE_OBSOLETES "grandorgue-resources grandorgue-demo")
  endif()

  # for source rpms
  set(CPACK_RPM_BUILDREQUIRES "pkgconfig(alsa), gcc-c++, jack-audio-connection-kit-devel, cmake, wxGTK3-devel, pkgconfig(fftw3f), pkgconfig(libudev), pkgconfig(wavpack), pkgconfig(zlib), libxslt, zip, po4a")
  set(CPACK_RPM_SOURCE_PKG_BUILD_PARAMS "-DVERSION=%{version} -DBUILD_VERSION=%{release}")

  # for deb
  set(CPACK_DEB_COMPONENT_INSTALL GO_SEPARATE_LINUX_PACKAGES)
  set(CPACK_DEBIAN_UNSPECIFIED_PACKAGE_NAME "grandorgue")
  set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
  set(CPACK_DEBIAN_PACKAGE_RELEASE "${CPACK_PACKAGE_RELEASE}")
  set(CPACK_DEBIAN_RESOURCES_PACKAGE_ARCHITECTURE all)
  set(CPACK_DEBIAN_DEMO_PACKAGE_ARCHITECTURE all)
  set(CPACK_DEBIAN_UNSPECIFIED_DEPENDS "grandorgue-resources")
  set(CPACK_DEBIAN_RESOURCES_DEPENDS "grandorgue")
  set(CPACK_DEBIAN_ENABLE_COMPONENT_DEPENDS ON)
  set(CPACK_DEBIAN_PACKAGE_SECTION sound)
  set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
  set(CPACK_DEBIAN_UNSPECIFIED_PACKAGE_RECOMMENDS "grandorgue-demo, wx3.0-i18n")
  set(CPACK_DEBIAN_PACKAGE_GENERATE_SHLIBS ON)
  set(CPACK_DEBIAN_PACKAGE_SOURCE "${CPACK_DEBIAN_UNSPECIFIED_PACKAGE_NAME}")
  if(NOT GO_SEPARATE_LINUX_PACKAGES)
    # For automatic removal of previously installed suppackages
    set(CPACK_DEBIAN_PACKAGE_REPLACES "grandorgue-resources, grandorgue-demo")
  endif()
endif()

set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CPACK_SYSTEM_NAME}.${CMAKE_SYSTEM_PROCESSOR}")

include(CPack)

message(STATUS "  Project                     : ${PROJECT_NAME}")
message(STATUS "  Description                 : ${CPACK_PACKAGE_DESCRIPTION_SUMMARY}")
message(STATUS "  Version                     : ${VERSION}")
message(STATUS "  Build                       : ${CPACK_PACKAGE_RELEASE}")
message(STATUS "============================================================================")
message(STATUS " ")
