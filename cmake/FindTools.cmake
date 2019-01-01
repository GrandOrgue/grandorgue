# GrandOrgue - free pipe organ simulator
# 
# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
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

include(FindGettext)
FIND_PROGRAM(GETTEXT_XGETTEXT_EXECUTABLE xgettext)
FIND_PROGRAM(XSLTPROC xsltproc)
FIND_PROGRAM(ZIP zip)
FIND_PROGRAM(PO4A_GETTEXT po4a-gettextize)
FIND_PROGRAM(PO4A_TRANSLATE po4a-translate)
FILE(GLOB DOCBOOK_SEARCH_DIR /usr/share/xml/docbook/xsl-stylesheets-*)
FIND_PATH(DOCBOOK_PATH htmlhelp/htmlhelp.xsl ${DOCBOOK_DIR} ${DOCBOOK_SEARCH_DIR} /usr/share/xml/docbook/stylesheet/nwalsh /usr/share/xml/docbook/stylesheet/nwalsh/current /usr/share/sgml/docbook/xsl-stylesheets CMAKE_FIND_ROOT_PATH_BOTH)

IF (NOT GETTEXT_FOUND)
   MESSAGE(STATUS "gettext not found")
ENDIF()
IF (NOT GETTEXT_XGETTEXT_EXECUTABLE)
   MESSAGE(STATUS "xgettext not found (package gettext)")
ENDIF()
IF (NOT GETTEXT_MSGMERGE_EXECUTABLE)
   MESSAGE(STATUS "msgmerge not found (package gettext)")
ENDIF()
IF (NOT GETTEXT_MSGFMT_EXECUTABLE)
   MESSAGE(STATUS "msgfmt not found (package gettext)")
ENDIF()
IF (NOT DOCBOOK_PATH)
   MESSAGE(STATUS "docbook-xsl stylesheets not found")
ENDIF()
IF (NOT XSLTPROC)
   MESSAGE(STATUS "xsltproc not found")
ENDIF()
IF (NOT ZIP)
   MESSAGE(STATUS "zip not found")
ENDIF()
IF (NOT PO4A_GETTEXT)
   MESSAGE(STATUS "po4a-gettextize not found (package po4a)")
ENDIF()
IF (NOT PO4A_TRANSLATE)
   MESSAGE(STATUS "po4a-translate not found (package po4a)")
ENDIF()

IF(CMAKE_CROSSCOMPILING)
  SET(IMPORT_EXECUTABLES "IMPORTFILE-NOTFOUND" CACHE FILEPATH "Point it to the export file from a native build")
  INCLUDE(${IMPORT_EXECUTABLES})
ENDIF(CMAKE_CROSSCOMPILING)
