# GrandOrgue - free pipe organ simulator
# 
# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2015 GrandOrgue contributors (see AUTHORS)
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

cmake_minimum_required(VERSION 2.6)

IF(NOT CMAKE_CROSSCOMPILING)
   ADD_EXECUTABLE(imageconverter imageconverter.cpp)
ENDIF(NOT CMAKE_CROSSCOMPILING)

IF(NOT CMAKE_CROSSCOMPILING) 
  EXPORT(TARGETS imageconverter FILE ${CMAKE_BINARY_DIR}/ImportExecutables.cmake )
ENDIF(NOT CMAKE_CROSSCOMPILING) 

