
set(CMAKE_RC_CREATE_STATIC_LIBRARY YES)

if (MINGW)
   set(CMAKE_RC_COMPILER_INIT windres)
   enable_language(RC)
   set(CMAKE_RC_COMPILE_OBJECT
      "<CMAKE_RC_COMPILER> -o <OBJECT> <SOURCE>")
endif ()

add_library(GrandOrgueResources STATIC GrandOrgue.rc)
