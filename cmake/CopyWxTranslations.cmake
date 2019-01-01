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

include(${CMAKE_SOURCE_DIR}/cmake/InstallTranslations.cmake)

function(CopyWxTranslations)
  set(searchdirs "")
  if(DEFINED wxWidgets_ROOT_DIR)
    get_filename_component(_dir ${wxWidgets_ROOT_DIR} REALPATH)
    if(IS_DIRECTORY ${_dir})
      list(APPEND searchdirs ${_dir})
    endif()
  endif()

  set(_libdirs)
  foreach(_file ${wxWidgets_LIBRARIES})
    string(REGEX REPLACE "^-L(.*)$" "\\1/dummy" _file ${_file})
    get_filename_component(dir ${_file} PATH)
    set(_libdirs ${_libdirs} ${dir})
  endforeach()

  foreach(dir ${wxWidgets_INCLUDE_DIRS} ${wxWidgets_LIBRARY_DIRS} ${_libdirs} $ENV{LC_PATH})
    foreach(postfix locale ../locale ../../locale  share/locale ../share/locale ../../share/locale ../../../share/locale)
      get_filename_component(_dir ${dir}/${postfix} REALPATH)
      if(IS_DIRECTORY ${_dir})
        list(APPEND searchdirs ${_dir})
      endif()
    endforeach()
  endforeach()
  list(REMOVE_DUPLICATES searchdirs)
  message(STATUS "wxTranslation search path: ${searchdirs}")
  InstallTranslations(wxstd ${searchdirs})
  InstallTranslations(wxmsw ${searchdirs})
endfunction()
