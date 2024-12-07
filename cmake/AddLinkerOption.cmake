# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later
# (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

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
