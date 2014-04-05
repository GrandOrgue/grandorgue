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
if (USE_INTERNAL_FFTW)
  include_directories(${CMAKE_BINARY_DIR}/lib/fftw/)
  set(FFTW_LIBRARIES fftw)
else()
  if(DEFINED FFTW_INCLUDE_DIRS)
    include_directories(${FFTW_INCLUDE_DIRS})
  endif()
  if(NOT DEFINED FFTW_LIBRARIES)
    set(FFTW_LIBRARIES fftw3f)
  endif()
endif()
if (USE_INTERNAL_WAVPACK)
  include_directories(../wavpack/include)
  set(WAVPACK_LIBRARIES WavPack)
else()
  if(DEFINED WAVPACK_INCLUDE_DIRS)
    include_directories(${WAVPACK_INCLUDE_DIRS})
  endif()
  if(NOT DEFINED WAVPACK_LIBRARIES)
    set(WAVPACK_LIBRARIES wavpack)
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
include_directories(${CMAKE_CURRENT_BINARY_DIR}/GrandOrgueDef.h ${CMAKE_CURRENT_SOURCE_DIR}/contrib)

CONVERTIMAGE(gauge.gif)
CONVERTIMAGE(help.png)
CONVERTIMAGE(memory.png)
CONVERTIMAGE(panic.png)
CONVERTIMAGE(properties.png)
CONVERTIMAGE(reload.png)
CONVERTIMAGE(set.png)
CONVERTIMAGE(transpose.png)
CONVERTIMAGE(reverb.png)
CONVERTIMAGE(open.png)
CONVERTIMAGE(polyphony.png)
CONVERTIMAGE(record.png)
CONVERTIMAGE(save.png)
CONVERTIMAGE(settings.png)
CONVERTIMAGE(volume.png)
CONVERTIMAGE(Splash.jpg)
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
CONVERTIMAGE(drawstop01off.png)
CONVERTIMAGE(drawstop01on.png)
CONVERTIMAGE(drawstop02off.png)
CONVERTIMAGE(drawstop02on.png)
CONVERTIMAGE(drawstop03off.png)
CONVERTIMAGE(drawstop03on.png)
CONVERTIMAGE(drawstop04off.png)
CONVERTIMAGE(drawstop04on.png)
CONVERTIMAGE(drawstop05off.png)
CONVERTIMAGE(drawstop05on.png)
CONVERTIMAGE(drawstop06off.png)
CONVERTIMAGE(drawstop06on.png)
CONVERTIMAGE(piston01off.png)
CONVERTIMAGE(piston01on.png)
CONVERTIMAGE(piston02off.png)
CONVERTIMAGE(piston02on.png)
CONVERTIMAGE(piston03off.png)
CONVERTIMAGE(piston03on.png)
CONVERTIMAGE(piston04off.png)
CONVERTIMAGE(piston04on.png)
CONVERTIMAGE(piston05off.png)
CONVERTIMAGE(piston05on.png)
CONVERTIMAGE(label01.png)
CONVERTIMAGE(label02.png)
CONVERTIMAGE(label03.png)
CONVERTIMAGE(label04.png)
CONVERTIMAGE(label05.png)
CONVERTIMAGE(label06.png)
CONVERTIMAGE(label07.png)
CONVERTIMAGE(label08.png)
CONVERTIMAGE(label09.png)
CONVERTIMAGE(label10.png)
CONVERTIMAGE(label11.png)
CONVERTIMAGE(label12.png)
CONVERTIMAGE(EnclosureA00.png)
CONVERTIMAGE(EnclosureA01.png)
CONVERTIMAGE(EnclosureA02.png)
CONVERTIMAGE(EnclosureA03.png)
CONVERTIMAGE(EnclosureA04.png)
CONVERTIMAGE(EnclosureA05.png)
CONVERTIMAGE(EnclosureA06.png)
CONVERTIMAGE(EnclosureA07.png)
CONVERTIMAGE(EnclosureA08.png)
CONVERTIMAGE(EnclosureA09.png)
CONVERTIMAGE(EnclosureA10.png)
CONVERTIMAGE(EnclosureA11.png)
CONVERTIMAGE(EnclosureA12.png)
CONVERTIMAGE(EnclosureA13.png)
CONVERTIMAGE(EnclosureA14.png)
CONVERTIMAGE(EnclosureA15.png)
CONVERTIMAGE(EnclosureB00.png)
CONVERTIMAGE(EnclosureB01.png)
CONVERTIMAGE(EnclosureB02.png)
CONVERTIMAGE(EnclosureB03.png)
CONVERTIMAGE(EnclosureB04.png)
CONVERTIMAGE(EnclosureB05.png)
CONVERTIMAGE(EnclosureB06.png)
CONVERTIMAGE(EnclosureB07.png)
CONVERTIMAGE(EnclosureB08.png)
CONVERTIMAGE(EnclosureB09.png)
CONVERTIMAGE(EnclosureB10.png)
CONVERTIMAGE(EnclosureB11.png)
CONVERTIMAGE(EnclosureB12.png)
CONVERTIMAGE(EnclosureB13.png)
CONVERTIMAGE(EnclosureB14.png)
CONVERTIMAGE(EnclosureB15.png)
CONVERTIMAGE(EnclosureC00.png)
CONVERTIMAGE(EnclosureC01.png)
CONVERTIMAGE(EnclosureC02.png)
CONVERTIMAGE(EnclosureC03.png)
CONVERTIMAGE(EnclosureC04.png)
CONVERTIMAGE(EnclosureC05.png)
CONVERTIMAGE(EnclosureC06.png)
CONVERTIMAGE(EnclosureC07.png)
CONVERTIMAGE(EnclosureC08.png)
CONVERTIMAGE(EnclosureC09.png)
CONVERTIMAGE(EnclosureC10.png)
CONVERTIMAGE(EnclosureC11.png)
CONVERTIMAGE(EnclosureC12.png)
CONVERTIMAGE(EnclosureC13.png)
CONVERTIMAGE(EnclosureC14.png)
CONVERTIMAGE(EnclosureC15.png)
CONVERTIMAGE(EnclosureD00.png)
CONVERTIMAGE(EnclosureD01.png)
CONVERTIMAGE(EnclosureD02.png)
CONVERTIMAGE(EnclosureD03.png)
CONVERTIMAGE(EnclosureD04.png)
CONVERTIMAGE(EnclosureD05.png)
CONVERTIMAGE(EnclosureD06.png)
CONVERTIMAGE(EnclosureD07.png)
CONVERTIMAGE(EnclosureD08.png)
CONVERTIMAGE(EnclosureD09.png)
CONVERTIMAGE(EnclosureD10.png)
CONVERTIMAGE(EnclosureD11.png)
CONVERTIMAGE(EnclosureD12.png)
CONVERTIMAGE(EnclosureD13.png)
CONVERTIMAGE(EnclosureD14.png)
CONVERTIMAGE(EnclosureD15.png)

set(grandorgue_src
contrib/sha1.cpp
contrib/zita-convolver.cpp

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
GOSoundReverb.cpp
GOSoundReverbEngine.cpp
GOSoundReverbPartition.cpp
GOSoundResample.cpp
GOSoundSamplerPool.cpp
GOSoundThread.cpp
GOrgueWave.cpp
GOrgueWavPack.cpp
GOrgueBitmap.cpp
GOrgueCache.cpp
GOrgueCacheWriter.cpp
GOrgueCombinationDefinition.cpp
GOrgueCombination.cpp
GOrgueConfigFileReader.cpp
GOrgueConfigFileWriter.cpp
GOrgueConfigReader.cpp
GOrgueConfigReaderDB.cpp
GOrgueConfigWriter.cpp
GOrgueBitmapCache.cpp
GOrgueButton.cpp
GOrgueCoupler.cpp
GOrgueDC.cpp
GOrgueDivisional.cpp
GOrgueDivisionalCoupler.cpp
GOrgueDummyPipe.cpp
GOrgueDrawStop.cpp
GOrgueEnclosure.cpp
GOrgueEvent.cpp
GOrgueFrameGeneral.cpp
GOrgueFont.cpp
GOrgueGeneral.cpp
GOrgueKeyConvert.cpp
GOrgueKeyReceiver.cpp
GOrgueKeyReceiverData.cpp
GOrgueLabel.cpp
GOrgueLCD.cpp
GOrgueLoadThread.cpp
GOrgueLog.cpp
GOrgueLogWindow.cpp
GOrgueManual.cpp
GOrgueMemoryPool.cpp
GOrgueMidi.cpp
GOrgueMidiEvent.cpp
GOrgueMidiFileReader.cpp
GOrgueMidiListener.cpp
GOrgueMidiMap.cpp
GOrgueMidiMerger.cpp
GOrgueMidiPlayer.cpp
GOrgueMidiSender.cpp
GOrgueMidiSenderData.cpp
GOrgueMidiReceiver.cpp
GOrgueMidiReceiverData.cpp
GOrgueMidiRecorder.cpp
GOrgueMidiWXEvent.cpp
GOrgueOrgan.cpp
GOrguePath.cpp
GOrguePipe.cpp
GOrguePipeConfig.cpp
GOrguePipeConfigNode.cpp
GOrguePipeConfigTreeNode.cpp
GOrguePiston.cpp
GOrgueProgressDialog.cpp
GOrgueProperties.cpp
GOrguePushbutton.cpp
GOrgueRank.cpp
GOrgueReferencePipe.cpp
GOrgueReleaseAlignTable.cpp
GOrgueRtHelpers.cpp
GOrgueSetter.cpp
GOrgueSetterButton.cpp
GOrgueSettings.cpp
GOrgueSound.cpp
GOrgueSoundingPipe.cpp
GOrgueStop.cpp
GOrgueTemperament.cpp
GOrgueTemperamentCent.cpp
GOrgueTremulant.cpp
GOrgueWindchest.cpp
GOrgueSwitch.cpp
GrandOrgueFile.cpp
GrandOrgueFrame.cpp
GOrgueUtil.cpp
MIDIEventDialog.cpp
MIDIEventRecvDialog.cpp
MIDIEventSendDialog.cpp
MIDIEventKeyDialog.cpp
MIDIList.cpp
OrganDialog.cpp
OrganSelectDialog.cpp
GOrgueDocument.cpp
GOrguePanelView.cpp
GOrgueView.cpp
SettingsAudioGroup.cpp
SettingsAudioOutput.cpp
SettingsDialog.cpp
SettingsMidiDevices.cpp
SettingsMidiMessage.cpp
SettingsOption.cpp
SettingsOrgan.cpp
SettingsReverb.cpp
SplashScreen.cpp
wxGaugeAudio.cpp
${IMAGE_LIST})

add_library(golib STATIC ${grandorgue_src})

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

   add_executable(${PROJECT_NAME} GrandOrgue.cpp)

endif ()

target_link_libraries(${PROJECT_NAME} golib ${wxWidgets_LIBRARIES} RtAudio RtMidi)
target_link_libraries(${PROJECT_NAME} ${PORTAUDIO_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${FFTW_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${WAVPACK_LIBRARIES})

if (APPLE)
   install(TARGETS ${PROJECT_NAME} BUNDLE DESTINATION . COMPONENT Runtime RUNTIME DESTINATION bin COMPONENT Runtime)
else (APPLE)
   install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
endif ()

if (INSTALL_DEPEND STREQUAL "ON")
  CopyWxTranslations()
  CopyDependencies(${PROJECT_NAME})
endif()

add_executable(perftest EXCLUDE_FROM_ALL perftest.cpp)
target_link_libraries(perftest golib ${wxWidgets_LIBRARIES} RtAudio RtMidi ${PORTAUDIO_LIBRARIES} ${FFTW_LIBRARIES} ${WAVPACK_LIBRARIES})

add_custom_target(test COMMAND perftest ${CMAKE_SOURCE_DIR}/tests DEPENDS perftest)
