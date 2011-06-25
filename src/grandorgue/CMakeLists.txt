include_directories(../rt/include)

# include wxWidgets
set(wxWidgets_USE_STATIC OFF)
set(wxWidgets_USE_LIBS base core net adv html)
include(CMakeWxWidgetsInclude.txt)

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
   add_subdirectory(resource bin/resource)
   add_executable(grandorgue WIN32 ${grandorgue_src})
   target_link_libraries(grandorgue GrandOrgueResources)
else ()
   add_executable(grandorgue ${grandorgue_src})
endif ()

target_link_libraries(grandorgue ${wxWidgets_LIBRARIES} RtAudio RtMidi)

