# GrandOrgue - free pipe organ simulator
# 
# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2016 GrandOrgue contributors (see AUTHORS)
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

include(${CMAKE_SOURCE_DIR}/cmake/BuildHelp.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/GetText.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/TranslateHelp.cmake)

file(GLOB PO_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.po)

if(XSLTPROC AND DOCBOOK_PATH AND ZIP)
  BUILD_HELPFILE(${CMAKE_CURRENT_SOURCE_DIR}/grandorgue.xml "")

  if(PO4A_GETTEXT AND PO4A_TRANSLATE)
    PO4A_BUILD_HELP(${CMAKE_CURRENT_SOURCE_DIR}/grandorgue.xml ${PO_FILES})
  else()
    MESSAGE(STATUS "Not building translations - some programs are missing")
  endif()
else()
   MESSAGE(STATUS "Not build help - some programs are missing")
endif()

PO4A_UPDATE_POT(help-update-pot grandorgue.pot grandorgue.xml)
GETTEXT_MERGE_PO(help-merge-pot grandorgue.pot ${PO_FILES})
