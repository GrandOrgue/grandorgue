# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later
# (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

include(CheckCXXCompilerFlag)

function(add_cxx_option _option)
  set(_name "OPTION_CXX_${_option}")
  string(REGEX REPLACE "=" "-EQ" _name ${_name})
  string(REGEX REPLACE "\\+" "-P" _name ${_name})
  SET(CMAKE_REQUIRED_FLAGS "${_option}")
  CHECK_CXX_COMPILER_FLAG("" ${_name})
  if(${_name})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${_option}" PARENT_SCOPE)
  endif()
endfunction()
