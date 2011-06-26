cmake_minimum_required(VERSION 2.6)

project(GrandOrgue CXX)

if (WIN32)
   # On Windows platforms, it is necessary to specify the root installation
   # path of wxWidgets.
   set(wxWidgets_ROOT_DIR C:/wxWidgets-2.8.12)
endif ()

# Build configuration options
if (NOT DEFINED UNICODE)
   set(UNICODE OFF)
endif ()
set(RTAUDIO_USE_JACK ON)
set(RTAUDIO_USE_OSS OFF)
set(RTAUDIO_USE_ALSA ON)
set(RTAUDIO_USE_DSOUND ON)
set(RTAUDIO_USE_ASIO ON)
set(RTMIDI_USE_JACK ON)
set(RTMIDI_USE_ALSA ON)
set(RTMIDI_USE_MM ON)

# Project version numbers
set(VERSION_MAJOR 0)
set(VERSION_MINOR 3)
set(VERSION_STAGE 0)
set(VERSION_REVISION 3)
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_STAGE}.${VERSION_REVISION})

if (CMAKE_BUILD_TYPE STREQUAL "")
   message(STATUS "No build configuration specified. Defaulting to 'Release' build.")
   set(CMAKE_BUILD_TYPE "Release")
endif ()

# setup compiler flags for debug vs release compiles
add_definitions("-fomit-frame-pointer -ffast-math -Wall")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
   add_definitions("-g -O0 -mmmx -msse -msse2 -msse3 -funit-at-a-time")
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
   add_definitions("-O2 -mmmx -msse -msse2 -msse3 -DNDEBUG")
endif ()

# include RtAudio
add_subdirectory(src/rt lib/rt)
add_subdirectory(src/grandorgue bin)

message(STATUS "  Project                     : ${PROJECT_NAME}")
message(STATUS "  Description                 : ${CPACK_PACKAGE_DESCRIPTION_SUMMARY}")
message(STATUS "  Version                     : ${VERSION}")
message(STATUS "============================================================================")
message(STATUS " ")
