# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later
# (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

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
