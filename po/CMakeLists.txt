# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later
# (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

include(${CMAKE_SOURCE_DIR}/cmake/GetText.cmake)

file(GLOB PO_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.po)
set(SRCLIST ${CMAKE_SOURCE_DIR}/src/*.cpp ${CMAKE_SOURCE_DIR}/src/*.h ${CMAKE_SOURCE_DIR}/resources/GrandOrgue.desktop.in)

IF (GETTEXT_FOUND)
  GETTEXT_UPDATE_POT(update-pot GrandOrgue.pot "${SRCLIST}" -k_ -kwxTRANSLATE --package-name=GrandOrgue --msgid-bugs-address="ourorgan-developers@lists.sourceforge.net" --foreign-user --no-location --from-code=ASCII)
  GETTEXT_MERGE_PO(merge-pot GrandOrgue.pot ${PO_FILES})
  GETTEXT_INSTALL_PO(GrandOrgue ${PO_FILES})
ENDIF()

