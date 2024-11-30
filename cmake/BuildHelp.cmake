# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later
# (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

function(BUILD_HELPFILE xmlfile VARIANT)
  SET(HELPDIR "${RESOURCEDIR}/help")
  SET(HELPINSTDIR "${RESOURCEINSTDIR}/help")

  SET(BUILDDIR ${CMAKE_CURRENT_BINARY_DIR}/help${VARIANT})
  SET(SRCDIR ${CMAKE_CURRENT_SOURCE_DIR})
  SET(IMGDIR ${SRCDIR}/images)
  SET(IMGLOCDIR ${SRCDIR}/images-${VARIANT})
  FILE(GLOB_RECURSE IMAGES RELATIVE ${IMGDIR} ${IMGDIR}/*.gif ${IMGDIR}/*.jpg ${IMGDIR}/*.png)
  file(MAKE_DIRECTORY ${BUILDDIR})
  file(MAKE_DIRECTORY ${HELPDIR})

  SET(_ImgList)
  FOREACH(IMG_NAME ${IMAGES})
    if(EXISTS ${IMGLOCDIR}/${IMG_NAME})
      set(_Img ${IMGLOCDIR}/${IMG_NAME})
    else()
      set(_Img ${IMGDIR}/${IMG_NAME})
    endif()
    ADD_CUSTOM_COMMAND(OUTPUT ${BUILDDIR}/images/${IMG_NAME} COMMAND ${CMAKE_COMMAND} -E copy ${_Img} ${BUILDDIR}/images/${IMG_NAME} DEPENDS ${_Img})
    SET(_ImgList ${_ImgList} ${BUILDDIR}/images/${IMG_NAME})
  ENDFOREACH()
  
  ADD_CUSTOM_COMMAND(OUTPUT ${BUILDDIR}/GrandOrgue.hhp COMMAND ${XSLTPROC}
    ARGS --path ${DOCBOOK_PATH}/htmlhelp ${SRCDIR}/grandorgue.xsl ${xmlfile}
    WORKING_DIRECTORY ${BUILDDIR} DEPENDS ${SRCDIR}/grandorgue.xsl ${xmlfile})

  ADD_CUSTOM_COMMAND(OUTPUT ${HELPDIR}/GrandOrgue${VARIANT}.htb COMMAND ${ZIP} 
    ARGS -r -X --filesync ${HELPDIR}/GrandOrgue${VARIANT}.htb * WORKING_DIRECTORY ${BUILDDIR}
    DEPENDS ${_ImgList} ${BUILDDIR}/GrandOrgue.hhp)

  INSTALL(FILES ${HELPDIR}/GrandOrgue${VARIANT}.htb DESTINATION ${HELPINSTDIR} COMPONENT resources)
  ADD_CUSTOM_TARGET(build-helpfile${VARIANT} DEPENDS ${HELPDIR}/GrandOrgue${VARIANT}.htb)

  IF (NOT TARGET build-help)
      ADD_CUSTOM_TARGET(build-help ALL)
  ENDIF ()
  ADD_DEPENDENCIES(build-help build-helpfile${VARIANT})
endfunction()
