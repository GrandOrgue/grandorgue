# GrandOrgue - free pipe organ simulator
# 
# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2015 GrandOrgue contributors (see AUTHORS)
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

message(STATUS "RtMidi build Configuration")

include_directories(../include)

add_library(RtMidi STATIC RtMidi.cpp)

if (WIN32)

   message(STATUS "  Building a Windows configuration")
   message(STATUS "    Support for MM            : ${RTMIDI_USE_MM}")
   message(STATUS "    Support for Jack          : ${RTMIDI_USE_JACK}")

   if (RTMIDI_USE_JACK)
      add_definitions(-D__UNIX_JACK__)
      target_link_libraries(RtMidi jack)
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
      target_link_libraries(RtMidi jack)
   endif ()

   if (RTMIDI_USE_ALSA)
      add_definitions(-D__LINUX_ALSA__)
      target_link_libraries(RtMidi asound)
   endif ()

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

   message(STATUS "  Building a Mac configuration")
   message(STATUS "    Support for CoreMIDI      : ${RTMIDI_USE_CORE}")
   message(STATUS "    Support for Jack          : ${RTMIDI_USE_JACK}")

   if (RTMIDI_USE_JACK)
      add_definitions(-D__UNIX_JACK__)
      target_link_libraries(RtMidi jack)
   endif ()

   if (RTMIDI_USE_CORE)
      add_definitions(-D__MACOSX_CORE__)
      target_link_libraries(RtMidi -framework\ CoreMIDI -framework\ CoreFoundation -framework\ CoreAudio)
   endif ()

else ()

   message(FATAL_ERROR "  Unknown/unsupported build configuration")

endif ()

message(STATUS "============================================================================")
