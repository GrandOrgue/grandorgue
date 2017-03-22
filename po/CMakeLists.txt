# GrandOrgue - free pipe organ simulator
# 
# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2017 GrandOrgue contributors (see AUTHORS)
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

include(${CMAKE_SOURCE_DIR}/cmake/GetText.cmake)

file(GLOB PO_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.po)
set(SRCLIST ${CMAKE_SOURCE_DIR}/src/*.cpp ${CMAKE_SOURCE_DIR}/src/*.h ${CMAKE_SOURCE_DIR}/resources/GrandOrgue.desktop.in)

IF (GETTEXT_FOUND)
  GETTEXT_UPDATE_POT(update-pot GrandOrgue.pot "${SRCLIST}" -k_ -kwxTRANSLATE --package-name=GrandOrgue --msgid-bugs-address="ourorgan-developers@lists.sourceforge.net" --foreign-user --no-location --from-code=ASCII)
  GETTEXT_MERGE_PO(merge-pot GrandOrgue.pot ${PO_FILES})
  GETTEXT_INSTALL_PO(GrandOrgue ${PO_FILES})
ENDIF()

