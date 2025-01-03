# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later
# (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

function(ExtractLibraryDirs result)
  set(dirlist ${ARGN})
  set(searchdirs "")
  set(_libdirs)
  foreach(_file ${dirlist})
    string(REGEX REPLACE "^-L(.*)$" "\\1/dummy" _file ${_file})
    get_filename_component(dir ${_file} PATH)
    set(_libdirs ${_libdirs} ${dir})
  endforeach()

  foreach(_dir ${dirlist} ${_libdirs})
    foreach(_prefix ${CMAKE_FIND_ROOT_PATH} ${CMAKE_SYSROOT} "")
      if(IS_DIRECTORY ${_prefix}/${_dir})
        list(APPEND searchdirs ${_dir})
      endif()
      if(IS_DIRECTORY ${_prefix}/${_dir}/../bin)
        list(APPEND searchdirs ${_dir}/../bin)
      endif()
    endforeach()
  endforeach()
  if(DEFINED DEPEND_ADD_DIRS)
    list(APPEND searchdirs ${DEPEND_ADD_DIRS})
  endif()
  list(REMOVE_DUPLICATES searchdirs)
  set(${result} ${searchdirs} PARENT_SCOPE)
endfunction()
