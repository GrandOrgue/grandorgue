# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later
# (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

include(${CMAKE_SOURCE_DIR}/cmake/InstallTranslations.cmake)

function(CopyWxTranslations)
  set(searchdirs "")
  if(DEFINED wxWidgets_ROOT_DIR)
    get_filename_component(_dir ${wxWidgets_ROOT_DIR} REALPATH)
    if(IS_DIRECTORY ${_dir})
      list(APPEND searchdirs ${_dir})
    endif()
  endif()

  set(_libdirs)
  foreach(_file ${wxWidgets_LIBRARIES})
    string(REGEX REPLACE "^-L(.*)$" "\\1/dummy" _file ${_file})
    get_filename_component(dir ${_file} PATH)
    set(_libdirs ${_libdirs} ${dir})
  endforeach()

  foreach(dir ${wxWidgets_INCLUDE_DIRS} ${wxWidgets_LIBRARY_DIRS} ${_libdirs} $ENV{LC_PATH})
    foreach(postfix locale ../locale ../../locale  share/locale ../share/locale ../../share/locale ../../../share/locale)
      get_filename_component(_dir ${dir}/${postfix} REALPATH)
      if(IS_DIRECTORY ${_dir})
        list(APPEND searchdirs ${_dir})
      endif()
    endforeach()
  endforeach()
  list(REMOVE_DUPLICATES searchdirs)
  message(STATUS "wxTranslation search path: ${searchdirs}")
  InstallTranslations(wxstd ${searchdirs})
  InstallTranslations(wxmsw ${searchdirs})
endfunction()
