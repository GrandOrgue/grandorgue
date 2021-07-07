# GrandOrgue - free pipe organ simulator
# 
# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
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

function(CONVERTIMAGE IMG_LIST imagefile)
  GET_FILENAME_COMPONENT(basename ${imagefile} NAME_WE)
  set(${IMG_LIST} ${${IMG_LIST}} ${basename}.cpp PARENT_SCOPE)
  set(IMAGE_DIR ${CMAKE_CURRENT_BINARY_DIR})
  file(MAKE_DIRECTORY ${IMAGE_DIR})
  if(IS_ABSOLUTE ${imagefile})
    set(IMAGE_FILE_ABS_PATH ${imagefile})
  else()
    set(IMAGE_FILE_ABS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${imagefile}")
  endif()
  ADD_CUSTOM_COMMAND(OUTPUT ${IMAGE_DIR}/${basename}.cpp
    COMMAND imageconverter ${IMAGE_FILE_ABS_PATH} ${IMAGE_DIR}/${basename}.cpp ${basename}
    DEPENDS ${IMAGE_FILE_ABS_PATH} imageconverter)
endfunction(CONVERTIMAGE)
