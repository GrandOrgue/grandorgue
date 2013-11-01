message(STATUS "RtAudio build Configuration")

INCLUDE (CheckIncludeFileCXX)

include_directories(../include)

set(RtAudio_Sources RtAudio.cpp)

if (WIN32)
   if (RTAUDIO_USE_ASIO)
      set(RtAudio_Sources ${RtAudio_Sources}
         ../../../ext/rt/asio/include/asiodrivers.cpp
         ../../../ext/rt/asio/include/asiolist.cpp
         ../../../ext/rt/asio/include/asio.cpp
         asiohelpers/include/iasiothiscallresolver.cpp)
   endif ()
endif ()

add_library(RtAudio STATIC ${RtAudio_Sources})

if (WIN32)

   message(STATUS "  Building a Windows configuration")
   message(STATUS "    Support for DirectSound   : ${RTAUDIO_USE_DSOUND}")
   message(STATUS "    Support for Asio          : ${RTAUDIO_USE_ASIO}")

   if (RTAUDIO_USE_DSOUND)
      add_definitions(-D__WINDOWS_DS__)
      include_directories(../../../ext/rt/directx)
      check_include_file_cxx(dsound.h DSOUND_H_EXISTS)
      if (${DSOUND_H_EXISTS})
      else()
	include_directories(../../../ext/rt/wine)
	include_directories(../../../ext/rt/wine/include)
      endif ()
      target_link_libraries(RtAudio dsound)
   endif ()

   if (RTAUDIO_USE_ASIO)
      add_definitions(-D__WINDOWS_ASIO__ -D_WIN32_WINNT=0x0501 -DWINVER=0x0501)
      include_directories(../../../ext/rt/asio)
      include_directories(../../../ext/rt/asio/include)
      include_directories(asiohelpers)
      include_directories(asiohelpers/include)
   endif ()

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

   message(STATUS "  Building a Linux configuration")
   message(STATUS "    Support for Jack          : ${RTAUDIO_USE_JACK}")
   message(STATUS "    Support for Alsa          : ${RTAUDIO_USE_ALSA}")
   message(STATUS "    Support for OSS           : ${RTAUDIO_USE_OSS}")

   if (RTAUDIO_USE_JACK)
      add_definitions(-D__UNIX_JACK__)
      target_link_libraries(RtAudio jack)
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
      target_link_libraries(RtAudio jackmp)
   endif ()

   if (RTAUDIO_USE_CORE)
      add_definitions(-D__MACOSX_CORE__)
      target_link_libraries(RtAudio -framework\ CoreAudio -framework\ CoreFoundation)
   endif ()

else ()

   message(FATAL_ERROR "  Unknown/unsupported build configuration")

endif ()

message(STATUS "============================================================================")
