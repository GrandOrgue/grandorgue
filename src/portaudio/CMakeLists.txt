# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

message(STATUS "PortAudio build Configuration")

INCLUDE (CheckIncludeFile)
INCLUDE (CheckCXXSourceCompiles)

include_directories(include)
include_directories(common)

CHECK_INCLUDE_FILE (sys/soundcard.h HAVE_SYS_SOUNDCARD_H)
if (HAVE_SYS_SOUNDCARD_H)
  add_definitions(-DHAVE_SYS_SOUNDCARD_H)
endif()


set(PortAudio_Sources 
  src/common/pa_allocation.c
  src/common/pa_converters.c
  src/common/pa_cpuload.c
  src/common/pa_debugprint.c
  src/common/pa_dither.c
  src/common/pa_front.c
  src/common/pa_process.c
  src/common/pa_ringbuffer.c
  src/common/pa_stream.c
  src/common/pa_trace.c)

set(PortAudio_Unix
  src/os/unix/pa_unix_hostapis.c
  src/os/unix/pa_unix_util.c)

set(PortAudio_Win
  src/os/win/pa_win_coinitialize.c
  src/os/win/pa_win_hostapis.c
  src/os/win/pa_win_util.c
  src/os/win/pa_win_waveformat.c)

set(PortAudio_Mac
  src/hostapi/coreaudio/pa_mac_core.c
  src/hostapi/coreaudio/pa_mac_core_blocking.c
  src/hostapi/coreaudio/pa_mac_core_utilities.c)

set(PA_Libs)

if (WIN32)

   message(STATUS "  Building a Windows configuration")
   message(STATUS "    Support for DirectSound   : ${RTAUDIO_USE_DSOUND}")
   message(STATUS "    Support for Asio          : ${RTAUDIO_USE_ASIO}")
   message(STATUS "    Support for WMME          : ${RTAUDIO_USE_WMME}")
   message(STATUS "    Support for WDMKS         : ${RTAUDIO_USE_WDMKS}")
   message(STATUS "    Support for WASAPI        : ${RTAUDIO_USE_WASAPI}")
   message(STATUS "    Support for Jack          : ${RTAUDIO_USE_JACK}")

   if (RTAUDIO_USE_DSOUND)
      add_definitions(-DPA_USE_DS)
      set(PortAudio_Sources ${PortAudio_Sources} src/hostapi/dsound/pa_win_ds.c src/hostapi/dsound/pa_win_ds_dynlink.c)
   endif ()

   if (RTAUDIO_USE_ASIO)
      add_definitions(-DPA_USE_ASIO -D_WIN32_WINNT=0x0501 -DWINVER=0x0501)
      include_directories(
	"${ASIO_SDK_DIR}/common"
        "${ASIO_SDK_DIR}/host"
	"${ASIO_SDK_DIR}/host/pc"
      )
      set(PortAudio_Sources ${PortAudio_Sources} src/hostapi/asio/iasiothiscallresolver.cpp src/hostapi/asio/pa_asio.cpp)
   endif ()

   if (RTAUDIO_USE_WMME)
      add_definitions(-DPA_USE_WMME)
      set(PortAudio_Sources ${PortAudio_Sources} src/hostapi/wmme/pa_win_wmme.c)
   endif ()

   check_include_file_cxx(rpcsal.h HAVE_RPCSAL_H)
   check_cxx_source_compiles("#include <ksmedia.h>
   KSRTAUDIO_HWREGISTER_PROPERTY x;" HAVE_KSRT)
   if (${HAVE_KSRT})
   else()
     include_directories(src/hostapi/wasapi/mingw-include)
   endif ()

   if (RTAUDIO_USE_WDMKS)
      add_definitions(-DPA_USE_WDMKS)
      set(PortAudio_Sources ${PortAudio_Sources} src/hostapi/wdmks/pa_win_wdmks.c)
      set(PA_Libs ${PA_Libs} setupapi)
   endif ()

   if (RTAUDIO_USE_WASAPI)
      add_definitions(-DPA_USE_WASAPI)
      set(PortAudio_Sources ${PortAudio_Sources} src/hostapi/wasapi/pa_win_wasapi.c)
   endif ()

   if (RTAUDIO_USE_JACK)
      add_definitions(-DPA_USE_JACK)
      set(PortAudio_Sources ${PortAudio_Sources} src/hostapi/jack/pa_jack.c)
   endif ()

   set(PortAudio_Sources ${PortAudio_Sources} ${PortAudio_Win})
   set(PA_Libs ${PA_Libs} winmm)
   include_directories(src/os/win)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

   message(STATUS "  Building a Linux configuration")
   message(STATUS "    Support for Jack          : ${RTAUDIO_USE_JACK}")
   message(STATUS "    Support for Alsa          : ${RTAUDIO_USE_ALSA}")
   message(STATUS "    Support for OSS           : ${RTAUDIO_USE_OSS}")

   if (RTAUDIO_USE_JACK)
      add_definitions(-DPA_USE_JACK)
      set(PortAudio_Sources ${PortAudio_Sources} src/hostapi/jack/pa_jack.c)
   endif ()

   if (RTAUDIO_USE_ALSA)
      add_definitions(-DPA_USE_ALSA)
      set(PortAudio_Sources ${PortAudio_Sources} src/hostapi/alsa/pa_linux_alsa.c)
      set(PA_Libs ${PA_Libs} asound)
   endif ()

   if (RTAUDIO_USE_OSS)
      add_definitions(-DPA_USE_OSS)
      set(PortAudio_Sources ${PortAudio_Sources} src/hostapi/oss/pa_unix_oss.c)
      set(PA_Libs ${PA_Libs} aoss)
   endif ()

   set(PortAudio_Sources ${PortAudio_Sources} ${PortAudio_Unix})
   include_directories(src/os/unix)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

   message(STATUS "  Building a Mac configuration")
   message(STATUS "    Support for Jack          : ${RTAUDIO_USE_JACK}")
   message(STATUS "    Support for CoreAudio     : ${RTAUDIO_USE_CORE}")

   set(PortAudio_Sources ${PortAudio_Sources} ${PortAudio_Unix})
   include_directories(src/os/unix)

   if (RTAUDIO_USE_JACK)
      add_definitions(-DPA_USE_JACK)
      set(PortAudio_Sources ${PortAudio_Sources} src/hostapi/jack/pa_jack.c)
   endif ()

   if (RTAUDIO_USE_CORE)
      add_definitions(-DPA_USE_COREAUDIO)
      set(PortAudio_Sources ${PortAudio_Sources} ${PortAudio_Mac})
      set(PA_Libs ${PA_Libs} -framework\ CoreAudio -framework\ CoreFoundation -framework\ AudioUnit)
   endif ()

else ()

   message(FATAL_ERROR "  Unknown/unsupported build configuration")

endif ()

add_library(PortAudio STATIC ${PortAudio_Sources})
target_link_libraries(PortAudio ${PA_Libs})
if (RTAUDIO_USE_JACK)
  include_directories(${JACK_INCLUDE_DIRS})
  target_link_libraries(PortAudio PkgConfig::JACK)
endif ()

message(STATUS "============================================================================")
