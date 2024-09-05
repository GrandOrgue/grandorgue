# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later
# (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

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
  HOMEPAGE_URL "https://github.com/GrandOrgue/grandorgue"
)
set(NUM_VERSION "${PROJECT_VERSION}")
string(REPLACE "." "," NUM_WIN_VERSION ${NUM_VERSION})

if(NOT DEFINED BUILD_VERSION)
  set(BUILD_VERSION "0.local")
elseif(NOT BUILD_VERSION)
  set(BUILD_VERSION "0.0")
endif()


if (NOT DEFINED GO_BUILD_TESTING OR GO_BUILD_TESTING STREQUAL "")
  set(GO_BUILD_TESTING ${BUILD_TESTING})
endif()

set(FULL_VERSION "${PROJECT_VERSION}-${BUILD_VERSION}")

# Set GITHUB_PROJECT variable that is used to configure update checking
if (NOT DEFINED GITHUB_PROJECT)
  set(GITHUB_PROJECT "GrandOrgue/grandorgue")
endif()

# Define a c++ standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17 CACHE STRING "The C++ standard whose features are requested to build this project")

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
option(USE_INTERNAL_RTAUDIO   "Use builtin RtAudio/RtMidi sources" ON)
option(INSTALL_DEMO           "Install demo sampleset" ON)
option(USE_INTERNAL_PORTAUDIO "Use builtin PortAudio sources" ON)
option(USE_INTERNAL_ZITACONVOLVER "Use builtin Zita Convolver sources" ON)
option(GO_USE_JACK	      "Use native Jack output" ON)
if (WIN32 OR APPLE)
   option(INSTALL_DEPEND      "Copy dependencies (wxWidgets libraries and Translations) on installation" ON)
else ()
   option(INSTALL_DEPEND      "Copy dependencies (wxWidgets libraries and Translations) on installation" OFF)
endif ()
option(GO_SEPARATE_LINUX_PACKAGES "Generate separate linux packages for resources and demo" OFF)
option(USE_BUILD_SYSTEM_LIBDIR "Use build system (distro-specific) libdir (Linux only)" OFF)

# only use options supported by the compiler
include(CheckIncludeFileCXX)
include(CheckFunctionExists)
include(FindPkgConfig)
include(${CMAKE_SOURCE_DIR}/cmake/AddOption.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/AddCXXOption.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/FindTools.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/BuildLibrary.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/BuildExecutable.cmake)

IF (APPLE)
  set(BININSTDIR "${PROJECT_NAME}.app/Contents/MacOS")
  SET(LIBINSTDIR "${PROJECT_NAME}.app/Contents/Frameworks")
  SET(RESOURCEINSTDIR "${PROJECT_NAME}.app/Contents/Resources")
  SET(RPATH_PREFIX "${PROJECT_NAME}.app/Contents/Frameworks")
ELSE()
  set(BININSTDIR "bin")
  IF (WIN32)
    SET(LIBINSTDIR "bin")
  ELSEIF (USE_BUILD_SYSTEM_LIBDIR)
    # See https://cmake.org/cmake/help/latest/module/GNUInstallDirs.html for
    # the purpose of this module
    include(GNUInstallDirs)
    SET(LIBINSTDIR "${CMAKE_INSTALL_LIBDIR}" CACHE STRING "library directory")
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
if (USE_INTERNAL_RTAUDIO)
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
else()
  pkg_check_modules(RTAUDIO REQUIRED rtaudio)
  pkg_check_modules(RTMIDI REQUIRED rtmidi)
  set(RT_LIBRARIES ${RTAUDIO_LIBRARIES} ${RTMIDI_LIBRARIES})
  set(RT_INCLUDE_DIRS ${RTMIDI_INCLUDE_DIRS} ${RTAUDIO_INCLUDE_DIRS})
endif()

# include portaudio
if (USE_INTERNAL_PORTAUDIO)
  set(PORTAUDIO_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/submodules/PortAudio/include)
  add_subdirectory(src/portaudio)
  set(PORTAUDIO_LIBRARIES PortAudio)
else()
  pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)
endif()

# include zitaconvolver
if (USE_INTERNAL_ZITACONVOLVER)
  set(ZITACONVOLVER_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/submodules/ZitaConvolver/source)
  set(ZITACONVOLVER_LIBRARIES "")
else()
  set(ZITACONVOLVER_INCLUDE_DIRS "")
  set(ZITACONVOLVER_LIBRARIES zita-convolver)
endif()

# include libcurl for automatic cheking for updates
# exports CURL::libcurl that can be used in target_link_libraries
find_package(CURL REQUIRED)

# include FFTW
pkg_check_modules(FFTW REQUIRED fftw3f)

pkg_check_modules(ZLIB REQUIRED zlib)

pkg_check_modules(WAVPACK REQUIRED wavpack)

find_package(wxWidgets REQUIRED base)
message(STATUS "  wxWidgets Unicode?          : ${wxWidgets_USE_UNICODE}")
message(STATUS "  wxWidgets Debug?            : ${wxWidgets_USE_DEBUG}")
message(STATUS "  wxWidgets Static linking    : ${wxWidgets_USE_STATIC}")
message(STATUS "  wxWidgets version           : ${wxWidgets_VERSION}")
message(STATUS "  wxWidgets config binary     : ${wxWidgets_CONFIG_EXECUTABLE}")
message(STATUS "  wxWidgets configuration     : ${wxWidgets_CONFIGURATION}")
message(STATUS "============================================================================")


if (GO_BUILD_TESTING)
    # Add coverage configuration to include project files - Don't put extra libraries here under
    set(CMAKE_CXX_FLAGS_DEBUG "-g --coverage")
    message(STATUS "============================================================================")
    message(STATUS " Coverage enabled")
    message(STATUS "============================================================================")
endif()

add_subdirectory(src/build)
add_subdirectory(src/images)
add_subdirectory(src/core)
add_subdirectory(src/grandorgue)
add_subdirectory(src/tools)
add_subdirectory(po)
add_subdirectory(help)
add_subdirectory(sounds)
add_subdirectory(packages)
add_subdirectory(perftests)
add_subdirectory(resources)

if (GO_BUILD_TESTING)
  enable_testing()
  add_subdirectory(src/tests)
  
  message(STATUS "============================================================================")
  message(STATUS " Testing enabled")
  message(STATUS "============================================================================")
endif()


# packaging

# deal with possible package names among grandorgue, grandorgue-wx30, grandorgue-wx32
set(BASE_PACKAGE_NAME "grandorgue")

set(OTHER_PACKAGE_NAMES "${BASE_PACKAGE_NAME}" "${BASE_PACKAGE_NAME}-wx30" "${BASE_PACKAGE_NAME}-wx32")
set(CPACK_PACKAGE_NAME "${BASE_PACKAGE_NAME}")
if(CMAKE_PACKAGE_SUFFIX)
  set(CPACK_PACKAGE_NAME "${BASE_PACKAGE_NAME}-${CMAKE_PACKAGE_SUFFIX}")
endif()
list(REMOVE_ITEM OTHER_PACKAGE_NAMES "${CPACK_PACKAGE_NAME}")
if(NOT GO_SEPARATE_LINUX_PACKAGES)
  # For automatic removal of previously installed subpackages
  list(APPEND OTHER_PACKAGE_NAMES "${BASE_PACKAGE_NAME}-resources" "${BASE_PACKAGE_NAME}-demo")
endif()

set(CPACK_PACKAGE_VENDOR "GrandOrgue contributors")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OpenSource Virtual Pipe Organ Software")
set(
  CPACK_PACKAGE_DESCRIPTION
  "GrandOrgue is a virtual pipe organ sample player application"
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

  add_custom_target(
    macOSApplication
    ALL
    DEPENDS GrandOrgue GrandOrgueTool GrandOrguePerfTest resources # run after building these targets
    COMMAND "${CMAKE_COMMAND}" -DAPP_DIR="${CMAKE_BINARY_DIR}" -P "${CMAKE_SOURCE_DIR}/cmake/SignMacOSApp.cmake"
  )

  set(CPACK_SYSTEM_NAME "macOS")
  set(CPACK_GENERATOR DragNDrop)
  set(CPACK_PRE_BUILD_SCRIPTS "${CMAKE_SOURCE_DIR}/cmake/SignMacOSApp.cmake")

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
  set(CPACK_RPM_PACKAGE_LICENSE "GPL-2.0-or-later")
  set(CPACK_RPM_PACKAGE_RELEASE "${CPACK_PACKAGE_RELEASE}")
  set(CPACK_RPM_PACKAGE_GROUP "Productivity/Multimedia/Sound/Midi")
  set(CPACK_RPM_PACKAGE_URL "https://github.com/GrandOrgue/grandorgue")
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
  string (REPLACE ";" " " CPACK_RPM_PACKAGE_OBSOLETES "${OTHER_PACKAGE_NAMES}")

  # On Ubuntu rpmbuild generates a libcurl.so.4(CURL_OPENSSL_4)(64bit) requirement for libcurl.
  # In Fedora libcurl is based on openssl, but libcurl package does not provide such symbol.
  # As a workaround, we set the right symbol manually.
  set(CPACK_RPM_SPEC_MORE_DEFINE "%global __requires_exclude libcurl.*")
  if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "aarch64" OR "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
    # dnf tries to install the 32-bit package on x86_64 if '()(64bit)' is omitted
    set(CPACK_RPM_PACKAGE_REQUIRES "libcurl.so.4()(64bit)")
  else()
    set(CPACK_RPM_PACKAGE_REQUIRES "libcurl.so.4")
  endif()

  # prevent rpmlint errors
  set(
    CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION
    /usr/share/icons
    /usr/share/man
    /usr/share/man/man1
  )

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
  set(CPACK_DEBIAN_UNSPECIFIED_PACKAGE_RECOMMENDS "grandorgue-demo")
  set(CPACK_DEBIAN_PACKAGE_GENERATE_SHLIBS ON)
  set(CPACK_DEBIAN_PACKAGE_SOURCE "${CPACK_DEBIAN_UNSPECIFIED_PACKAGE_NAME}")
  string (REPLACE ";" ", " CPACK_DEBIAN_PACKAGE_REPLACES "${OTHER_PACKAGE_NAMES}")
endif()

set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CPACK_SYSTEM_NAME}.${CMAKE_SYSTEM_PROCESSOR}")

include(CPack)

message(STATUS "  Project                     : ${PROJECT_NAME}")
message(STATUS "  Description                 : ${CPACK_PACKAGE_DESCRIPTION_SUMMARY}")
message(STATUS "  Version                     : ${VERSION}")
message(STATUS "  Build                       : ${CPACK_PACKAGE_RELEASE}")
message(STATUS "  Package name                : ${CPACK_PACKAGE_NAME}")
message(STATUS "============================================================================")
message(STATUS " ")
