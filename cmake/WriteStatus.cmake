# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later
# (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

function(WriteStatus statusfile)
  get_cmake_property(_varlist VARIABLES)

  set(_list "")
  foreach(_var ${_varlist})
    string (REGEX REPLACE "([\\\\\"])" "\\\\\\1" _val "${${_var}}")
    set (_list "${_list}set(${_var} \"${_val}\")\n")
  endforeach()

  file(WRITE "${statusfile}" "${_list}")
endfunction()