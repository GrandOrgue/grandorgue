# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later
# (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

include(CheckCCompilerFlag)

function(add_option _option)
  set(_name "OPTION_C_${_option}")
  string(REGEX REPLACE "=" "-EQ" _name ${_name})
  string(REGEX REPLACE "\\+" "-P" _name ${_name})
  CHECK_C_COMPILER_FLAG("${_option}" ${_name})
  if(${_name})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${_option}" PARENT_SCOPE)
  endif()
  CHECK_CXX_COMPILER_FLAG("${_option}" ${_name}_cxx)
  if(${_name}_cxx)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${_option}" PARENT_SCOPE)
  endif()
endfunction(add_option option)
