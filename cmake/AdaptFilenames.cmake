# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later
# (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

FILE(READ ${infile} f0 )
STRING(REPLACE "#.wav" "s.wav" f1 "${f0}" )
FILE(WRITE ${outfile} "${f1}")
