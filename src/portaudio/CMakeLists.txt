message(STATUS "PortAudio build Configuration")

INCLUDE (CheckIncludeFile)

include_directories(include)
include_directories(common)

CHECK_INCLUDE_FILE (sys/soundcard.h HAVE_SYS_SOUNDCARD_H)
if (HAVE_SYS_SOUNDCARD_H)
  add_definitions(-DHAVE_SYS_SOUNDCARD_H)
endif()


set(PortAudio_Sources 
  common/pa_allocation.c
  common/pa_converters.c
  common/pa_cpuload.c
  common/pa_debugprint.c
  common/pa_dither.c
  common/pa_front.c
  common/pa_process.c
  common/pa_ringbuffer.c
  common/pa_stream.c
  common/pa_trace.c)

set(PortAudio_Unix
  os/unix/pa_unix_hostapis.c
  os/unix/pa_unix_util.c)

set(PortAudio_Win
  os/win/pa_win_coinitialize.c
  os/win/pa_win_hostapis.c
  os/win/pa_win_util.c
  os/win/pa_win_waveformat.c)

set(PA_Libs)

if (WIN32)

   message(STATUS "  Building a Windows configuration")
   message(STATUS "    Support for DirectSound   : ${RTAUDIO_USE_DSOUND}")
   message(STATUS "    Support for Asio          : ${RTAUDIO_USE_ASIO}")
   message(STATUS "    Support for WMME          : ${RTAUDIO_USE_WMME}")
   message(STATUS "    Support for WDMKS         : ${RTAUDIO_USE_WDMKS}")
   message(STATUS "    Support for WASAPI        : ${RTAUDIO_USE_WASAPI}")

   if (RTAUDIO_USE_DSOUND)
      add_definitions(-DPA_USE_DS)
      set(PortAudio_Sources ${PortAudio_Sources} hostapi/dsound/pa_win_ds.c hostapi/dsound/pa_win_ds_dynlink.c)
      include_directories(../../ext/rt/directx)
      check_include_file_cxx(dsound.h DSOUND_H_EXISTS)
      if (${DSOUND_H_EXISTS})
      else()
	include_directories(../../ext/rt/wine)
	include_directories(../../ext/rt/wine/include)
      endif ()
   endif ()

   if (RTAUDIO_USE_ASIO)
      add_definitions(-DPA_USE_ASIO)
      include_directories(../../ext/rt/asio)
      include_directories(../../ext/rt/asio/include)
      set(PortAudio_Sources ${PortAudio_Sources} hostapi/asio/iasiothiscallresolver.cpp hostapi/asio/pa_asio.cpp)
   endif ()

   if (RTAUDIO_USE_WMME)
      add_definitions(-DPA_USE_WMME)
      set(PortAudio_Sources ${PortAudio_Sources} hostapi/wmme/pa_win_wmme.c)
   endif ()

   check_include_file_cxx(rpcsal.h HAVE_RPCSAL_H)
   if (${HAVE_RPCSAL_H})
   else()
     include_directories(hostapi/wasapi/mingw-include)
     include_directories(hostapi/wasapi/wine-include)
   endif ()

   if (RTAUDIO_USE_WDMKS)
      add_definitions(-DPA_USE_WDMKS)
      set(PortAudio_Sources ${PortAudio_Sources} hostapi/wdmks/pa_win_wdmks.c)
      set(PA_Libs ${PA_libs} setupapi)
   endif ()

   if (RTAUDIO_USE_WASAPI)
      add_definitions(-DPA_USE_WASAPI)
      set(PortAudio_Sources ${PortAudio_Sources} hostapi/wasapi/pa_win_wasapi.c)
   endif ()

   set(PortAudio_Sources ${PortAudio_Sources} ${PortAudio_Win})
   include_directories(os/win)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

   message(STATUS "  Building a Linux configuration")
   message(STATUS "    Support for Jack          : ${RTAUDIO_USE_JACK}")
   message(STATUS "    Support for Alsa          : ${RTAUDIO_USE_ALSA}")
   message(STATUS "    Support for OSS           : ${RTAUDIO_USE_OSS}")

   if (RTAUDIO_USE_JACK)
      add_definitions(-DPA_USE_JACK)
      set(PortAudio_Sources ${PortAudio_Sources} hostapi/jack/pa_jack.c)
      set(PA_Libs ${PA_libs} jack)
   endif ()

   if (RTAUDIO_USE_ALSA)
      add_definitions(-DPA_USE_ALSA)
      set(PortAudio_Sources ${PortAudio_Sources} hostapi/alsa/pa_linux_alsa.c)
      set(PA_Libs ${PA_libs} asound)
   endif ()

   if (RTAUDIO_USE_OSS)
      add_definitions(-DPA_USE_OSS)
      set(PortAudio_Sources ${PortAudio_Sources} hostapi/oss/pa_unix_oss.c)
      set(PA_Libs ${PA_libs} aoss)
   endif ()

   set(PortAudio_Sources ${PortAudio_Sources} ${PortAudio_Unix})
   include_directories(os/unix)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

   message(STATUS "  Building a Mac configuration")
   message(STATUS "    Support for Jack          : ${RTAUDIO_USE_JACK}")
   message(STATUS "    Support for CoreAudio     : ${RTAUDIO_USE_CORE}")

   set(PortAudio_Sources ${PortAudio_Sources} ${PortAudio_Unix})
   include_directories(os/unix)

   if (RTAUDIO_USE_JACK)
      add_definitions(-DPA_USE_JACK)
      set(PortAudio_Sources ${PortAudio_Sources} hostapi/jack/pa_jack.c)
      set(PA_Libs ${PA_libs} jackmp)
   endif ()

   if (RTAUDIO_USE_CORE)
      add_definitions(-DPA_USE_COREAUDIO)
      set(PortAudio_Sources ${PortAudio_Sources} hostapi/coreaudio/pa_mac_core.c hostapi/coreaudio/pa_mac_blocking.c hostapi/coreaudio/pa_mac_utilities.c)
      set(PA_Libs ${PA_libs} -framework\ CoreAudio -framework\ CoreFoundation)
   endif ()

else ()

   message(FATAL_ERROR "  Unknown/unsupported build configuration")

endif ()

add_library(PortAudio STATIC ${PortAudio_Sources})
target_link_libraries(PortAudio ${PA_Libs})

message(STATUS "============================================================================")
