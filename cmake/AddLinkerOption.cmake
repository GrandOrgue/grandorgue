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

function(add_linker_option _target _option)
  set(_name "OPTION_LINK_${_option}")
  SET(CMAKE_REQUIRED_FLAGS "-Wl,--${_option}")
  CHECK_CXX_COMPILER_FLAG("" ${_name})
  if(${_name})
    set(new_flags "-Wl,--${_option}")
    get_target_property(existing_flags ${_target} LINK_FLAGS)
    if(existing_flags)
      set(new_flags "${existing_flags} ${new_flags}")
    endif()
    set_target_properties(${_target} PROPERTIES LINK_FLAGS ${new_flags})
  endif()
endfunction()
