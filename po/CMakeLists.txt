include(FindGettext)
include(CMakeGetText.txt)

file(GLOB_RECURSE SRC_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp ${CMAKE_SOURCE_DIR}/src/*.h)
foreach(FILENAME ${SRC_FILES})
	file(APPEND ${CMAKE_BINARY_DIR}/po/source_files.txt "${FILENAME}\n")
endforeach(FILENAME)
file(GLOB PO_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.po)

GETTEXT_UPDATE_POT(update-pot GrandOrgue.pot -k_ -kwxTRANSLATE --package-name=GrandOrgue --msgid-bugs-address="ourorgan-developers@lists.sourceforge.net" --foreign-user --no-location --from-code=ASCII --files-from ${CMAKE_BINARY_DIR}/po/source_files.txt)
GETTEXT_MERGE_PO(merge-pot GrandOrgue.pot ${PO_FILES})

GETTEXT_INSTALL_PO(GrandOrgue ${PO_FILES})

