# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

message(STATUS "RtAudio build Configuration")

INCLUDE (CheckIncludeFileCXX)

set(RtAudio_Sources "${RTAUDIO_SRC_DIR}/RtAudio.cpp")

if (WIN32)
  if (RTAUDIO_USE_ASIO)
    add_definitions(-D__WINDOWS_ASIO__ -D_WIN32_WINNT=0x0501 -DWINVER=0x0501)
    # they should be included before ${RTAUDIO_SRC_DIR}/include because 
    # the same headers in ${RTAUDIO_SRC_DIR}/include are of a wrong version
    include_directories(
      "${ASIO_SDK_DIR}/common"
      "${ASIO_SDK_DIR}/host"
      "${ASIO_SDK_DIR}/host/pc"
    )
    set(RtAudio_Sources ${RtAudio_Sources}
      "${ASIO_SDK_DIR}/host/asiodrivers.cpp"
      "${ASIO_SDK_DIR}/host/pc/asiolist.cpp"
      "${ASIO_SDK_DIR}/common/asio.cpp"
      "${RTAUDIO_SRC_DIR}/include/iasiothiscallresolver.cpp"
    )
  endif ()
endif ()

include_directories("${RTAUDIO_SRC_DIR}/include")
add_library(RtAudio STATIC ${RtAudio_Sources})

if (WIN32)

   message(STATUS "  Building a Windows configuration")
   message(STATUS "    Support for DirectSound   : ${RTAUDIO_USE_DSOUND}")
   message(STATUS "    Support for Asio          : ${RTAUDIO_USE_ASIO}")

   if (RTAUDIO_USE_DSOUND)
      add_definitions(-D__WINDOWS_DS__)
      target_link_libraries(RtAudio dsound)
   endif ()

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

   message(STATUS "  Building a Linux configuration")
   message(STATUS "    Support for Jack          : ${RTAUDIO_USE_JACK}")
   message(STATUS "    Support for Alsa          : ${RTAUDIO_USE_ALSA}")
   message(STATUS "    Support for OSS           : ${RTAUDIO_USE_OSS}")

   if (RTAUDIO_USE_JACK)
      add_definitions(-D__UNIX_JACK__)
      target_link_libraries(RtAudio PkgConfig::JACK)
   endif ()

   if (RTAUDIO_USE_ALSA)
      add_definitions(-D__LINUX_ALSA__)
      target_link_libraries(RtAudio asound)
   endif ()

   if (RTAUDIO_USE_OSS)
      add_definitions(-D__LINUX_OSS__)
      target_link_libraries(RtAudio aoss)
   endif ()

   target_link_libraries(RtAudio pthread)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

   message(STATUS "  Building a Mac configuration")
   message(STATUS "    Support for Jack          : ${RTAUDIO_USE_JACK}")
   message(STATUS "    Support for CoreAudio     : ${RTAUDIO_USE_CORE}")

   if (RTAUDIO_USE_JACK)
      add_definitions(-D__UNIX_JACK__)
      target_link_libraries(RtAudio PkgConfig::JACK)
   endif ()

   if (RTAUDIO_USE_CORE)
      add_definitions(-D__MACOSX_CORE__)
      target_link_libraries(RtAudio -framework\ CoreAudio -framework\ CoreFoundation)
   endif ()

else ()

   message(FATAL_ERROR "  Unknown/unsupported build configuration")

endif ()

message(STATUS "============================================================================")
