# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

include_directories(${JACK_INCLUDE_DIRS})

# pull in RtAudio
add_subdirectory(rtaudio)

# pull in RtMidi
add_subdirectory(rtmidi)

