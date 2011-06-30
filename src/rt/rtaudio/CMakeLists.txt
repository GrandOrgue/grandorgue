include_directories(../include)

add_library(RtAudio STATIC RtAudio.cpp)

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



