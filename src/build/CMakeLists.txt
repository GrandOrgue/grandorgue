# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

cmake_minimum_required(VERSION 3.10)

IF(NOT CMAKE_CROSSCOMPILING)
   ADD_EXECUTABLE(imageconverter imageconverter.cpp)
ENDIF(NOT CMAKE_CROSSCOMPILING)

IF(NOT CMAKE_CROSSCOMPILING) 
  EXPORT(TARGETS imageconverter FILE ${CMAKE_BINARY_DIR}/ImportExecutables.cmake )
ENDIF(NOT CMAKE_CROSSCOMPILING) 

