# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later
# (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

# Wraps a text file into a C++ raw string constant.
# Called in script mode from EMBEDTEXTFILE below:
#   cmake -DINPUT=... -DOUTPUT=... -DSYMBOL=... -P EmbedTextFile.cmake
if(CMAKE_SCRIPT_MODE_FILE)
  file(READ ${INPUT} CONTENT)
  file(WRITE ${OUTPUT}
    "// generated from ${INPUT}, do not edit\n"
    "extern const char *const ${SYMBOL};\n"
    "const char *const ${SYMBOL} = R\"gotext(${CONTENT})gotext\";\n")
else()
  function(EMBEDTEXTFILE SRC_LIST textfile symbol)
    set(out ${CMAKE_CURRENT_BINARY_DIR}/${symbol}.cpp)
    set(${SRC_LIST} ${${SRC_LIST}} ${out} PARENT_SCOPE)
    add_custom_command(OUTPUT ${out}
      COMMAND ${CMAKE_COMMAND}
        -DINPUT=${CMAKE_CURRENT_SOURCE_DIR}/${textfile}
        -DOUTPUT=${out} -DSYMBOL=${symbol}
        -P ${CMAKE_SOURCE_DIR}/cmake/EmbedTextFile.cmake
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${textfile} ${CMAKE_SOURCE_DIR}/cmake/EmbedTextFile.cmake)
  endfunction()
endif()
