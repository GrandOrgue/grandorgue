message(STATUS "RtMidi build Configuration")

include_directories(../include)

add_library(RtMidi STATIC RtMidi.cpp)

if (WIN32)

   message(STATUS "  Building a Windows configuration")
   message(STATUS "    Support for MM            : ${RTMIDI_USE_MM}")

   if (RTMIDI_USE_MM)
      add_definitions(-D__WINDOWS_MM__)
#      include_directories(../../../ext/rt/directx)
      target_link_libraries(RtMidi winmm comdlg32 shell32 comctl32 advapi32 ole32 oleaut32 uuid)
   endif ()

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

   message(STATUS "  Building a Linux configuration")
   message(STATUS "    Support for Jack          : ${RTMIDI_USE_JACK}")
   message(STATUS "    Support for Alsa          : ${RTMIDI_USE_ALSA}")

   if (RTMIDI_USE_JACK)
      add_definitions(-D__UNIX_JACK__)
      target_link_libraries(RtMidi jack)
   endif ()

   if (RTMIDI_USE_ALSA)
      add_definitions(-D__LINUX_ALSA__)
      target_link_libraries(RtMidi asound)
   endif ()

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

   message(STATUS "  Building a Mac configuration")
   message(STATUS "    Support for CoreMIDI      : ${RTMIDI_USE_CORE}")

   if (RTMIDI_USE_CORE)
      add_definitions(-D__MACOSX_CORE__)
      target_link_libraries(RtMidi -framework\ CoreMIDI -framework\ CoreFoundation -framework\ CoreAudio)
   endif ()

else ()

   message(FATAL_ERROR "  Unknown/unsupported build configuration")

endif ()

message(STATUS "============================================================================")
