include_directories(../rt/include)

# include wxWidgets
set(wxWidgets_USE_STATIC OFF)
include(CMakeWxWidgetsInclude.txt)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/GrandOrgueDef.h.in ${CMAKE_CURRENT_BINARY_DIR}/GrandOrgueDef.h/GrandOrgueDef.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/GrandOrgueDef.h)

set(grandorgue_src
contrib/sha1.cpp
GOrgueWave.cpp
GOrgueControl.cpp
GOrgueCoupler.cpp
GOrgueDisplayMetrics.cpp
GOrgueDivisional.cpp
GOrgueDivisionalCoupler.cpp
GOrgueDrawStop.cpp
GOrgueEnclosure.cpp
GOrgueFrameGeneral.cpp
GOrgueGeneral.cpp
GOrgueLabel.cpp
GOrgueLCD.cpp
GOrgueManual.cpp
GOrgueMidi.cpp
GOrgueMidiEvent.cpp
GOrgueMeter.cpp
GOrguePipe.cpp
GOrguePiston.cpp
GOrgueProperties.cpp
GOrguePushbutton.cpp
GOrgueReleaseAlignTable.cpp
GOrgueRtHelpers.cpp
GOrgueSound.cpp
GOrgueSoundCallbackAudio.cpp
GOrgueStop.cpp
GOrgueTremulant.cpp
GOrgueWindchest.cpp
GrandOrgue.cpp
GrandOrgueFile.cpp
GrandOrgueFrame.cpp
Images.cpp
IniFileConfig.cpp
MIDIListenDialog.cpp
OrganDocument.cpp
OrganPanel.cpp
OrganView.cpp
SettingsDialog.cpp
SplashScreen.cpp
wxGaugeAudio.cpp)

if (WIN32)

   if (MINGW)

      set(CMAKE_RC_COMPILER_INIT windres)
      enable_language(RC)

      if (CMAKE_BUILD_TYPE STREQUAL "Debug")
         set(rc_debug_flag "")
      else ()
         set(rc_debug_flag "-DNDEBUG")
      endif ()

      set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> ${rc_debug_flag} -O coff -D_WIN32 -I${CMAKE_CURRENT_SOURCE_DIR}/resource -i <SOURCE> -o <OBJECT>")

   endif ()

   configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resource/GrandOrgue.rc.in ${CMAKE_CURRENT_BINARY_DIR}/GrandOrgue.rc/GrandOrgue.rc)
   set(grandorgue_src ${grandorgue_src} ${CMAKE_CURRENT_BINARY_DIR}/GrandOrgue.rc/GrandOrgue.rc)
   add_executable(${PROJECT_NAME} WIN32 ${grandorgue_src})

else ()

   add_executable(${PROJECT_NAME} ${grandorgue_src})

endif ()

target_link_libraries(${PROJECT_NAME} ${wxWidgets_LIBRARIES} RtAudio RtMidi)

