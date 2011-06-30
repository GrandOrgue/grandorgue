include_directories(../include)

add_library(RtMidi STATIC RtMidi.cpp)

if (RTMIDI_USE_JACK)
   add_definitions(-D__UNIX_JACK__)
   target_link_libraries(RtMidi jack)
endif ()

if (RTMIDI_USE_ALSA)
   add_definitions(-D__LINUX_ALSASEQ__)
   target_link_libraries(RtMidi asound)
endif ()



