FILE(READ ${infile} f0 )
STRING(REPLACE "#.wav" "s.wav" f1 "${f0}" )
FILE(WRITE ${outfile} "${f1}")
