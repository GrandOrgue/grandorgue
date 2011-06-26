include_directories(../rt/include)

# include wxWidgets
set(wxWidgets_USE_STATIC OFF)
set(wxWidgets_USE_LIBS base core net adv html)
include(CMakeWxWidgetsInclude.txt)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/GrandOrgueDef.h.in ${CMAKE_CURRENT_BINARY_DIR}/GrandOrgueDef.h/GrandOrgueDef.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/GrandOrgueDef.h)

set(grandorgue_src
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
      set(CMAKE_RC_COMPILE_OBJECT
         "<CMAKE_RC_COMPILER> -I${CMAKE_CURRENT_SOURCE_DIR}/resource -o <OBJECT> <SOURCE>")
   endif ()

endif ()

if (WIN32)
   set(grandorgue_src ${grandorgue_src} resource/GrandOrgue.rc)
   add_executable(grandorgue WIN32 ${grandorgue_src})
else ()
   add_executable(grandorgue ${grandorgue_src})
endif ()

target_link_libraries(grandorgue ${wxWidgets_LIBRARIES} RtAudio RtMidi)

