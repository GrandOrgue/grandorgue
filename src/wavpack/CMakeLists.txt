message(STATUS "wavpack build Configuration")

include_directories(src/include)

add_definitions(-DNO_USE_FSTREAMS)
add_definitions(-DNO_TAGS)
add_definitions(-DNO_SEEKING)
add_definitions(-DVER4_ONLY)

add_library(WavPack STATIC 
   src/common_utils.c
   src/decorr_utils.c
   src/entropy_utils.c
   src/open_utils.c
   src/read_words.c
   src/unpack.c
   src/unpack_floats.c
   src/unpack_utils.c
  )
