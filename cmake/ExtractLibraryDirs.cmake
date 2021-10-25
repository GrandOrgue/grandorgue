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

function(ExtractLibraryDirs result)
  set(dirlist ${ARGN})
  set(searchdirs "")
  set(_libdirs)
  foreach(_file ${dirlist})
    string(REGEX REPLACE "^-L(.*)$" "\\1/dummy" _file ${_file})
    get_filename_component(dir ${_file} PATH)
    set(_libdirs ${_libdirs} ${dir})
  endforeach()

  foreach(_dir ${dirlist} ${_libdirs})
    foreach(_prefix ${CMAKE_FIND_ROOT_PATH} ${CMAKE_SYSROOT} "")
      if(IS_DIRECTORY ${_prefix}/${_dir})
        list(APPEND searchdirs ${_dir})
      endif()
      if(IS_DIRECTORY ${_prefix}/${_dir}/../bin)
        list(APPEND searchdirs ${_dir}/../bin)
      endif()
    endforeach()
  endforeach()
  if(DEFINED DEPEND_ADD_DIRS)
    list(APPEND searchdirs ${DEPEND_ADD_DIRS})
  endif()
  list(REMOVE_DUPLICATES searchdirs)
  set(${result} ${searchdirs} PARENT_SCOPE)
endfunction()
