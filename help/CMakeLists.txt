# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later
# (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

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
