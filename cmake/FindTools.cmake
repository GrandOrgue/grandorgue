# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later
# (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

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

FIND_FILE(XML_CATALOG_FILE catalog
  PATHS
    /opt/homebrew/etc/xml   # macOS Homebrew arm64
    /usr/local/etc/xml      # macOS Homebrew x86
    /etc/xml                # Linux
  NO_DEFAULT_PATH
)
IF (NOT XML_CATALOG_FILE)
   MESSAGE(STATUS "XML catalog not found - xsltproc may fail to resolve DocBook entities")
ENDIF()

IF(CMAKE_CROSSCOMPILING)
  SET(IMPORT_EXECUTABLES "IMPORTFILE-NOTFOUND" CACHE FILEPATH "Point it to the export file from a native build")
  INCLUDE(${IMPORT_EXECUTABLES})
ENDIF(CMAKE_CROSSCOMPILING)
