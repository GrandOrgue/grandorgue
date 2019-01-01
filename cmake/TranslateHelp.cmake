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

function(PO4A_BUILD_HELP xmlfile)
  FOREACH(_currentPoFile ${ARGN})
    GET_FILENAME_COMPONENT(_lang ${_currentPoFile} NAME_WE)
    ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_lang}.xml COMMAND ${PO4A_TRANSLATE}
      ARGS -f docbook -m ${xmlfile} -p ${_currentPoFile} -l ${CMAKE_CURRENT_BINARY_DIR}/${_lang}.xml -k 0
      DEPENDS ${_currentPoFile} ${xmlfile})
          BUILD_HELPFILE(${CMAKE_CURRENT_BINARY_DIR}/${_lang}.xml _${_lang})
  ENDFOREACH()
endfunction()

function(PO4A_UPDATE_POT target _potFile xmlfile)
  ADD_CUSTOM_TARGET(${target} COMMAND ${PO4A_GETTEXT} ARGS -f docbook -m ${xmlfile} -p ${CMAKE_CURRENT_BINARY_DIR}/${_potFile} --package-name=GrandOrgue --msgid-bugs-address="ourorgan-developers@lists.sourceforge.net" WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endfunction()

