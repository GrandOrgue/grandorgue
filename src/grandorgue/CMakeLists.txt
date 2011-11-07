include_directories(../rt/include)

# include wxWidgets
if (STATIC)
  set(wxWidgets_USE_STATIC ON)
else()
  set(wxWidgets_USE_STATIC OFF)
endif()

include(CMakeWxWidgetsInclude.txt)
include(CMakeImages.txt)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/GrandOrgueDef.h.in ${CMAKE_CURRENT_BINARY_DIR}/GrandOrgueDef.h/GrandOrgueDef.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/GrandOrgueDef.h)

CONVERTIMAGE(gauge.gif)
CONVERTIMAGE(draw1off.png)
CONVERTIMAGE(draw2off.png)
CONVERTIMAGE(fpiston2.png)
CONVERTIMAGE(help.png)
CONVERTIMAGE(hpiston.png)
CONVERTIMAGE(memory.png)
CONVERTIMAGE(panic.png)
CONVERTIMAGE(properties.png)
CONVERTIMAGE(reload.png)
CONVERTIMAGE(set.png)
CONVERTIMAGE(transpose.png)
CONVERTIMAGE(draw1on.png)
CONVERTIMAGE(draw2on.png)
CONVERTIMAGE(fpiston.png)
CONVERTIMAGE(hpiston2.png)
CONVERTIMAGE(label.png)
CONVERTIMAGE(open.png)
CONVERTIMAGE(polyphony.png)
CONVERTIMAGE(record.png)
CONVERTIMAGE(save.png)
CONVERTIMAGE(settings.png)
CONVERTIMAGE(volume.png)
CONVERTIMAGE(Splash.jpg)
CONVERTIMAGE(Wood01.jpg)
CONVERTIMAGE(Wood03.jpg)
CONVERTIMAGE(Wood05.jpg)
CONVERTIMAGE(Wood07.jpg)
CONVERTIMAGE(Wood09.jpg)
CONVERTIMAGE(Wood11.jpg)
CONVERTIMAGE(Wood13.jpg)
CONVERTIMAGE(Wood15.jpg)
CONVERTIMAGE(Wood17.jpg)
CONVERTIMAGE(Wood19.jpg)
CONVERTIMAGE(Wood21.jpg)
CONVERTIMAGE(Wood23.jpg)
CONVERTIMAGE(Wood25.jpg)
CONVERTIMAGE(Wood27.jpg)
CONVERTIMAGE(Wood29.jpg)
CONVERTIMAGE(Wood31.jpg)
CONVERTIMAGE(Wood33.jpg)
CONVERTIMAGE(Wood35.jpg)
CONVERTIMAGE(Wood37.jpg)
CONVERTIMAGE(Wood39.jpg)
CONVERTIMAGE(Wood41.jpg)
CONVERTIMAGE(Wood43.jpg)
CONVERTIMAGE(Wood45.jpg)
CONVERTIMAGE(Wood47.jpg)
CONVERTIMAGE(Wood49.jpg)
CONVERTIMAGE(Wood51.jpg)
CONVERTIMAGE(Wood53.jpg)
CONVERTIMAGE(Wood55.jpg)
CONVERTIMAGE(Wood57.jpg)
CONVERTIMAGE(Wood59.jpg)
CONVERTIMAGE(Wood61.jpg)
CONVERTIMAGE(Wood63.jpg)
CONVERTIMAGE(GOIcon.ico)

set(grandorgue_src
contrib/sha1.cpp
GOGUIControl.cpp
GOGUIDisplayMetrics.cpp
GOGUIHW1DisplayMetrics.cpp
GOGUISetterDisplayMetrics.cpp
GOGUIDrawStop.cpp
GOGUIEnclosure.cpp
GOGUIHW1Background.cpp
GOGUILabel.cpp
GOGUIManual.cpp
GOGUIManualBackground.cpp
GOGUIPanel.cpp
GOGUIPanelWidget.cpp
GOGUIPushbutton.cpp
GOGUISetterButton.cpp
GOSoundEngine.cpp
GOSoundProvider.cpp
GOSoundProviderSynthedTrem.cpp
GOSoundProviderWave.cpp
GOSoundRecorder.cpp
GOSoundSamplerPool.cpp
GOSoundThread.cpp
GOrgueWave.cpp
GOrgueCache.cpp
GOrgueCacheWriter.cpp
GOrgueControl.cpp
GOrgueCoupler.cpp
GOrgueDivisional.cpp
GOrgueDivisionalCoupler.cpp
GOrgueDrawStop.cpp
GOrgueEnclosure.cpp
GOrgueEvent.cpp
GOrgueFrameGeneral.cpp
GOrgueGeneral.cpp
GOrgueLabel.cpp
GOrgueLCD.cpp
GOrgueManual.cpp
GOrgueMemoryPool.cpp
GOrgueMidi.cpp
GOrgueMidiEvent.cpp
GOrgueMidiReceiver.cpp
GOrguePipe.cpp
GOrguePiston.cpp
GOrgueProperties.cpp
GOrguePushbutton.cpp
GOrgueReleaseAlignTable.cpp
GOrgueRtHelpers.cpp
GOrgueSetter.cpp
GOrgueSetterButton.cpp
GOrgueSettings.cpp
GOrgueSound.cpp
GOrgueStop.cpp
GOrgueTremulant.cpp
GOrgueWindchest.cpp
GrandOrgue.cpp
GrandOrgueFile.cpp
GrandOrgueFrame.cpp
Images.cpp
IniFileConfig.cpp
MIDIEventDialog.cpp
MIDIListenDialog.cpp
OrganDocument.cpp
OrganView.cpp
SettingsDialog.cpp
SplashScreen.cpp
wxGaugeAudio.cpp
${IMAGE_LIST})

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

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
