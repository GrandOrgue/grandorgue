include(CMakeHtmlHelp.txt)

file(GLOB PO_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.po)

BUILD_HELPFILE(help-main ${BUILD_HELP} ${CMAKE_CURRENT_SOURCE_DIR}/grandorgue.xml "")
PO4A_UPDATE_POT(help-update-pot grandorgue.pot grandorgue.xml)
GETTEXT_MERGE_PO(help-merge-po grandorgue.pot ${PO_FILES})
PO4A_BUILD_HELP(help-translate ${CMAKE_CURRENT_SOURCE_DIR}/grandorgue.xml ${PO_FILES})
