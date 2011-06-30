cmake_minimum_required(VERSION 2.6)

project(GrandOrgue CXX)

set(UNICODE OFF)

set(RTAUDIO_USE_JACK ON)
set(RTAUDIO_USE_OSS OFF)
set(RTAUDIO_USE_ALSA ON)
set(RTMIDI_USE_JACK ON)
set(RTMIDI_USE_ALSA ON)

if (NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE Release)
endif ()

# setup compiler flags for debug vs release compiles
add_definitions("-fomit-frame-pointer -ffast-math -Wall")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
   add_definitions("-g -O0 -mmmx -msse -msse2 -msse3 -funit-at-a-time")
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
   add_definitions("-O2 -mmmx -msse -msse2 -msse3 -DNDEBUG")
endif ()

# include RtAudio
add_subdirectory(src/rt bin)
add_subdirectory(src/grandorgue bin)

message(STATUS "================================================================================")
message(STATUS " ")
message(STATUS "  Project                     : ${PROJECT_NAME}")
message(STATUS "  Description                 : ${CPACK_PACKAGE_DESCRIPTION_SUMMARY}")
message(STATUS "  Version                     : ${VERSION}")
message(STATUS " ")
message(STATUS "  wxWidgets version           : ${wxWidgets_VERSION}")
message(STATUS "  wxWidgets config binary     : ${wxWidgets_CONFIG_EXECUTABLE}")
message(STATUS "  wxWidgets Static linking    : ${wxWidgets_USE_STATIC}")
message(STATUS "  wxWidgets Unicode?          : ${wxWidgets_USE_UNICODE}")
message(STATUS "  wxWidgets Debug?            : ${wxWidgets_USE_DEBUG}")
message(STATUS "================================================================================")
message(STATUS " ")

