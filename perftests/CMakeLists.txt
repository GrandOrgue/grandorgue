# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later
# (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

include(${CMAKE_SOURCE_DIR}/cmake/CopyStructure.cmake)

file(MAKE_DIRECTORY ${RESOURCEDIR}/perftests)
install(DIRECTORY DESTINATION ${RESOURCEINSTDIR}/perftests)
COPY_STRUCTURE(perftests "perftests" resources)
