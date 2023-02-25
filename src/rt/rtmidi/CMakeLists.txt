# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

message(STATUS "RtMidi build Configuration")

include_directories("${RTMIDI_SRC_DIR}")
# not more necessary for the recent RtMidi
# include_directories(../../core/contrib)

add_library(RtMidi STATIC "${RTMIDI_SRC_DIR}/RtMidi.cpp")

if (WIN32)

   message(STATUS "  Building a Windows configuration")
   message(STATUS "    Support for MM            : ${RTMIDI_USE_MM}")
   message(STATUS "    Support for Jack          : ${RTMIDI_USE_JACK}")

   if (RTMIDI_USE_JACK)
      add_definitions(-D__UNIX_JACK__)
      target_link_libraries(RtMidi PkgConfig::JACK)
   endif ()

   if (RTMIDI_USE_MM)
      add_definitions(-D__WINDOWS_MM__)
      target_link_libraries(RtMidi winmm comdlg32 shell32 comctl32 advapi32 ole32 oleaut32 uuid)
   endif ()

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

   message(STATUS "  Building a Linux configuration")
   message(STATUS "    Support for Jack          : ${RTMIDI_USE_JACK}")
   message(STATUS "    Support for Alsa          : ${RTMIDI_USE_ALSA}")

   if (RTMIDI_USE_JACK)
      add_definitions(-D__UNIX_JACK__)
      target_link_libraries(RtMidi PkgConfig::JACK)
   endif ()

   if (RTMIDI_USE_ALSA)
      add_definitions(-D__LINUX_ALSA__)
      target_link_libraries(RtMidi asound udev)
      set(CMAKE_REQUIRED_LIBRARIES asound)
      check_function_exists(snd_seq_client_info_get_card HAVE_GET_CLIENT_INFO_CARD)
      if (HAVE_GET_CLIENT_INFO_CARD)
        add_definitions(-DHAVE_GET_CLIENT_INFO_CARD)
      endif ()
   endif ()

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

   message(STATUS "  Building a Mac configuration")
   message(STATUS "    Support for CoreMIDI      : ${RTMIDI_USE_CORE}")
   message(STATUS "    Support for Jack          : ${RTMIDI_USE_JACK}")

   if (RTMIDI_USE_JACK)
      add_definitions(-D__UNIX_JACK__)
      target_link_libraries(RtMidi PkgConfig::JACK)
   endif ()

   if (RTMIDI_USE_CORE)
      add_definitions(-D__MACOSX_CORE__)
      target_link_libraries(RtMidi -framework\ CoreMIDI -framework\ CoreFoundation -framework\ CoreAudio)
   endif ()

else ()

   message(FATAL_ERROR "  Unknown/unsupported build configuration")

endif ()

message(STATUS "============================================================================")
