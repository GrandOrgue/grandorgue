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

set(DEPLIST)

if (APPLE)
   ADD_CUSTOM_COMMAND(OUTPUT "${RESOURCEDIR}/GrandOrgue.icns" COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/resource/GrandOrgue.icns "${RESOURCEDIR}/GrandOrgue.icns" DEPENDS ${CMAKE_SOURCE_DIR}/src/grandorgue/resource/GrandOrgue.icns)
   INSTALL(FILES "${RESOURCEDIR}/GrandOrgue.icns" DESTINATION "${RESOURCEINSTDIR}")
   LIST(ADD DEPLIST "${RESOURCEDIR}/GrandOrgue.icns")
elseif(UNIX)
  install(FILES GrandOrgue.png DESTINATION share/pixmaps)
  install(FILES grandorgue.xml DESTINATION share/mime/packages)
endif()

ADD_CUSTOM_TARGET(resources ALL DEPENDS ${DEPLIST})
