# GrandOrgue - free pipe organ simulator
# 
# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2018 GrandOrgue contributors (see AUTHORS)
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

message(STATUS "wavpack build Configuration")

include_directories(src/include)

add_definitions(-DNO_USE_FSTREAMS)
add_definitions(-DNO_TAGS)
add_definitions(-DNO_SEEKING)
add_definitions(-DVER4_ONLY)

add_library(WavPack STATIC 
   src/common_utils.c
   src/decorr_utils.c
   src/entropy_utils.c
   src/open_utils.c
   src/read_words.c
   src/unpack.c
   src/unpack_floats.c
   src/unpack_utils.c
  )
