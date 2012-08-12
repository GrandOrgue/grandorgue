include_directories(../rt/include)
if (USE_INTERNAL_PORTAUDIO)
  include_directories(../portaudio/include)
  set(PORTAUDIO_LIBRARIES PortAudio)
else()
  if(DEFINED PORTAUDIO_INCLUDE_DIRS)
    include_directories(${PORTAUDIO_INCLUDE_DIRS})
  endif()
  if(NOT DEFINED PORTAUDIO_LIBRARIES)
    set(PORTAUDIO_LIBRARIES portaudio)
  endif()
endif()

# include wxWidgets
if (STATIC)
  set(wxWidgets_USE_STATIC ON)
else()
  set(wxWidgets_USE_STATIC OFF)
endif()

include(CMakeWxWidgetsInclude.txt)
include(CMakeImages.txt)
include(CMakeWxBundle.txt)

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
CONVERTIMAGE(reverb.png)
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
CONVERTIMAGE(Enclosure0.png)
CONVERTIMAGE(Enclosure1.png)
CONVERTIMAGE(Enclosure2.png)
CONVERTIMAGE(Enclosure3.png)
CONVERTIMAGE(Enclosure4.png)
CONVERTIMAGE(Enclosure5.png)
CONVERTIMAGE(Enclosure6.png)
CONVERTIMAGE(Enclosure7.png)
CONVERTIMAGE(Enclosure8.png)
CONVERTIMAGE(Enclosure9.png)
CONVERTIMAGE(Enclosure10.png)
CONVERTIMAGE(Enclosure11.png)
CONVERTIMAGE(Enclosure12.png)
CONVERTIMAGE(Enclosure13.png)
CONVERTIMAGE(Enclosure14.png)
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
CONVERTIMAGE(ManualCBlackDown.png)
CONVERTIMAGE(ManualCBlackUp.png)
CONVERTIMAGE(ManualCWhiteDown.png)
CONVERTIMAGE(ManualCWhiteUp.png)
CONVERTIMAGE(ManualCWoodDown.png)
CONVERTIMAGE(ManualCWoodUp.png)
CONVERTIMAGE(ManualDBlackDown.png)
CONVERTIMAGE(ManualDBlackUp.png)
CONVERTIMAGE(ManualDWhiteDown.png)
CONVERTIMAGE(ManualDWhiteUp.png)
CONVERTIMAGE(ManualDWoodDown.png)
CONVERTIMAGE(ManualDWoodUp.png)
CONVERTIMAGE(ManualEBlackDown.png)
CONVERTIMAGE(ManualEBlackUp.png)
CONVERTIMAGE(ManualEWhiteDown.png)
CONVERTIMAGE(ManualEWhiteUp.png)
CONVERTIMAGE(ManualEWoodDown.png)
CONVERTIMAGE(ManualEWoodUp.png)
CONVERTIMAGE(ManualNaturalBlackDown.png)
CONVERTIMAGE(ManualNaturalBlackUp.png)
CONVERTIMAGE(ManualNaturalWhiteDown.png)
CONVERTIMAGE(ManualNaturalWhiteUp.png)
CONVERTIMAGE(ManualNaturalWoodDown.png)
CONVERTIMAGE(ManualNaturalWoodUp.png)
CONVERTIMAGE(ManualSharpBlackDown.png)
CONVERTIMAGE(ManualSharpBlackUp.png)
CONVERTIMAGE(ManualSharpWhiteDown.png)
CONVERTIMAGE(ManualSharpWhiteUp.png)
CONVERTIMAGE(ManualSharpWoodDown.png)
CONVERTIMAGE(ManualSharpWoodUp.png)
CONVERTIMAGE(PedalNaturalBlackDown.png)
CONVERTIMAGE(PedalNaturalBlackUp.png)
CONVERTIMAGE(PedalNaturalWoodDown.png)
CONVERTIMAGE(PedalNaturalWoodUp.png)
CONVERTIMAGE(PedalSharpBlackDown.png)
CONVERTIMAGE(PedalSharpBlackUp.png)
CONVERTIMAGE(PedalSharpWoodDown.png)
CONVERTIMAGE(PedalSharpWoodUp.png)

set(grandorgue_src
contrib/sha1.cpp
GOGUIButton.cpp
GOGUIControl.cpp
GOGUIDisplayMetrics.cpp
GOGUIHW1DisplayMetrics.cpp
GOGUISetterDisplayMetrics.cpp
GOGUIEnclosure.cpp
GOGUIHW1Background.cpp
GOGUIImage.cpp
GOGUILabel.cpp
GOGUIManual.cpp
GOGUIManualBackground.cpp
GOGUIPanel.cpp
GOGUIPanelWidget.cpp
GOSoundAudioSection.cpp
GOSoundEngine.cpp
GOSoundProvider.cpp
GOSoundProviderSynthedTrem.cpp
GOSoundProviderWave.cpp
GOSoundRecorder.cpp
GOSoundResample.cpp
GOSoundSamplerPool.cpp
GOSoundThread.cpp
GOrgueWave.cpp
GOrgueCache.cpp
GOrgueCacheWriter.cpp
GOrgueCombinationDefinition.cpp
GOrgueCombination.cpp
GOrgueConfigReader.cpp
GOrgueConfigWriter.cpp
GOrgueBitmapCache.cpp
GOrgueButton.cpp
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
GOrgueMidiSender.cpp
GOrgueMidiReceiver.cpp
GOrguePath.cpp
GOrguePipe.cpp
GOrguePipeConfig.cpp
GOrguePiston.cpp
GOrgueProperties.cpp
GOrguePushbutton.cpp
GOrgueRank.cpp
GOrgueReleaseAlignTable.cpp
GOrgueRtHelpers.cpp
GOrgueSetter.cpp
GOrgueSetterButton.cpp
GOrgueSettings.cpp
GOrgueSound.cpp
GOrgueStop.cpp
GOrgueTemperament.cpp
GOrgueTemperamentCent.cpp
GOrgueTremulant.cpp
GOrgueWindchest.cpp
GrandOrgueFile.cpp
GrandOrgueFrame.cpp
Images.cpp
IniFileConfig.cpp
MIDIEventDialog.cpp
MIDIListenDialog.cpp
OrganDialog.cpp
OrganDocument.cpp
OrganView.cpp
SettingsAudioGroup.cpp
SettingsAudioOutput.cpp
SettingsDialog.cpp
SettingsMidiDevices.cpp
SettingsMidiMessage.cpp
SettingsOption.cpp
SettingsOrgan.cpp
SplashScreen.cpp
wxGaugeAudio.cpp
${IMAGE_LIST})

macro(add_linker_option _target _option)
  set(_name "OPTION_LINK_${_option}")
  SET(CMAKE_REQUIRED_FLAGS "-Wl,--${_option}")
  CHECK_CXX_COMPILER_FLAG("" ${_name})
  if(${_name})
    set(new_flags "-Wl,--${_option}")
    get_target_property(existing_flags ${_target} LINK_FLAGS)
    if(existing_flags)
      set(new_flags "${existing_flags} ${new_flags}")
    endif()
    set_target_properties(${_target} PROPERTIES LINK_FLAGS ${new_flags})
  endif()
endmacro()

if (WIN32)

   if (MINGW)

      set(CMAKE_RC_COMPILER_INIT windres)
      enable_language(RC)

      if (CMAKE_BUILD_TYPE STREQUAL "Debug")
         set(rc_debug_flag "")
      else ()
         set(rc_debug_flag "-DNDEBUG")
      endif ()

      set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> ${CMAKE_RC_FLAGS} ${rc_debug_flag} -O coff -D_WIN32 -I${CMAKE_CURRENT_SOURCE_DIR}/resource -i <SOURCE> -o <OBJECT>")

   endif ()

   configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resource/GrandOrgue.rc.in ${CMAKE_CURRENT_BINARY_DIR}/GrandOrgue.rc/GrandOrgue.rc)
   set(grandorgue_src ${grandorgue_src} ${CMAKE_CURRENT_BINARY_DIR}/GrandOrgue.rc/GrandOrgue.rc)
   add_executable(${PROJECT_NAME} WIN32 GrandOrgue.cpp ${grandorgue_src})
   add_linker_option(${PROJECT_NAME} large-address-aware)

elseif (APPLE)

   configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resource/Info.plist.in ${CMAKE_CURRENT_BINARY_DIR}/temp.plist)
   configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resource/GrandOrgue.icns ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.app/Contents/Resources/${PROJECT_NAME}.icns COPYONLY)
   add_executable(${PROJECT_NAME} MACOSX_BUNDLE GrandOrgue.cpp ${grandorgue_src})
   set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
   set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_BINARY_DIR}/temp.plist)

else ()

   add_executable(${PROJECT_NAME} GrandOrgue.cpp ${grandorgue_src})

endif ()

target_link_libraries(${PROJECT_NAME} ${wxWidgets_LIBRARIES} RtAudio RtMidi)
target_link_libraries(${PROJECT_NAME} ${PORTAUDIO_LIBRARIES})

if (APPLE)
   install(TARGETS ${PROJECT_NAME} BUNDLE DESTINATION ../../.. COMPONENT Runtime RUNTIME DESTINATION bin COMPONENT Runtime)
else (APPLE)
   install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
endif ()

if (INSTALL_DEPEND STREQUAL "ON")
  CopyWxTranslations()
  CopyDependencies(${PROJECT_NAME})
endif()

add_executable(perftest EXCLUDE_FROM_ALL perftest.cpp ${grandorgue_src})
target_link_libraries(perftest ${wxWidgets_LIBRARIES} RtAudio RtMidi ${PORTAUDIO_LIBRARIES})

add_custom_target(test COMMAND perftest ${CMAKE_SOURCE_DIR}/tests DEPENDS perftest)
