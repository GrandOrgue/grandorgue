# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later
# (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

include(${CMAKE_SOURCE_DIR}/cmake/AddCXXOption.cmake)

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "x86")
add_cxx_option(-mmmx)
add_cxx_option(-msse)
add_cxx_option(-msse2)
add_cxx_option(-msse3)
endif()

add_cxx_option(-mstackrealign)

