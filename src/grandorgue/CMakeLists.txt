
# include wxWidgets
if (STATIC)
  set(wxWidgets_USE_STATIC ON)
else()
  set(wxWidgets_USE_STATIC OFF)
endif()

include(CMakeWxWidgetsInclude.txt)
include(CMakeWxBundle.txt)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/GrandOrgueDef.h.in ${CMAKE_CURRENT_BINARY_DIR}/GrandOrgueDef.h/GrandOrgueDef.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/GrandOrgueDef.h ${CMAKE_CURRENT_SOURCE_DIR}/contrib)

add_subdirectory(images)

set(grandorgue_src
contrib/sha1.cpp
contrib/zita-convolver.cpp

GOGUIBankedGeneralsPanel.cpp
GOGUIButton.cpp
GOGUICrescendoPanel.cpp
GOGUIControl.cpp
GOGUICouplerPanel.cpp
GOGUIDisplayMetrics.cpp
GOGUIDivisionalsPanel.cpp
GOGUILayoutEngine.cpp
GOGUIHW1DisplayMetrics.cpp
GOGUISetterDisplayMetrics.cpp
GOGUIEnclosure.cpp
GOGUIFloatingPanel.cpp
GOGUIHW1Background.cpp
GOGUIImage.cpp
GOGUILabel.cpp
GOGUIManual.cpp
GOGUIManualBackground.cpp
GOGUIMasterPanel.cpp
GOGUIMetronomePanel.cpp
GOGUIPanel.cpp
GOGUIPanelWidget.cpp
GOGUISequencerPanel.cpp
GOSoundAudioSection.cpp
GOSoundEngine.cpp
GOSoundGroupWorkItem.cpp
GOSoundOutputWorkItem.cpp
GOSoundProvider.cpp
GOSoundProviderSynthedTrem.cpp
GOSoundProviderWave.cpp
GOSoundRecorder.cpp
GOSoundReverb.cpp
GOSoundReverbEngine.cpp
GOSoundReverbPartition.cpp
GOSoundResample.cpp
GOSoundSamplerPool.cpp
GOSoundScheduler.cpp
GOSoundThread.cpp
GOSoundTremulantWorkItem.cpp
GOSoundWindchestWorkItem.cpp
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
GOrgueEventDistributor.cpp
GOrgueFrameGeneral.cpp
GOrgueFont.cpp
GOrgueGeneral.cpp
GOrgueHash.cpp
GOrgueInvalidFile.cpp
GOrgueKeyConvert.cpp
GOrgueKeyReceiver.cpp
GOrgueKeyReceiverData.cpp
GOrgueLabel.cpp
GOrgueLoadThread.cpp
GOrgueLog.cpp
GOrgueLogWindow.cpp
GOrgueManual.cpp
GOrgueMemoryPool.cpp
GOrgueMetronome.cpp
GOrgueMidi.cpp
GOrgueMidiEvent.cpp
GOrgueMidiFileReader.cpp
GOrgueMidiListener.cpp
GOrgueMidiMap.cpp
GOrgueMidiMerger.cpp
GOrgueMidiInPort.cpp
GOrgueMidiOutPort.cpp
GOrgueMidiPlayer.cpp
GOrgueMidiSender.cpp
GOrgueMidiSenderData.cpp
GOrgueMidiReceiver.cpp
GOrgueMidiReceiverData.cpp
GOrgueMidiRecorder.cpp
GOrgueMidiRtInPort.cpp
GOrgueMidiRtOutPort.cpp
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
GOrgueStandardFile.cpp
GOrgueSetter.cpp
GOrgueSetterButton.cpp
GOrgueSetting.cpp
GOrgueSettingBool.cpp
GOrgueSettingDirectory.cpp
GOrgueSettingFile.cpp
GOrgueSettingFloat.cpp
GOrgueSettingStore.cpp
GOrgueSettingString.cpp
GOrgueSettings.cpp
GOrgueSound.cpp
GOrgueSoundPort.cpp
GOrgueSoundPortaudioPort.cpp
GOrgueSoundRtPort.cpp
GOrgueSoundingPipe.cpp
GOrgueStdPath.cpp
GOrgueStop.cpp
GOrgueTemperament.cpp
GOrgueTemperamentCent.cpp
GOrgueTemperamentList.cpp
GOrgueTemperamentUser.cpp
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
SettingsDefaults.cpp
SettingsDialog.cpp
SettingsMidiDevices.cpp
SettingsMidiMessage.cpp
SettingsOption.cpp
SettingsOrgan.cpp
SettingsReverb.cpp
SettingsTemperaments.cpp
SplashScreen.cpp
wxGaugeAudio.cpp)

add_library(golib STATIC ${grandorgue_src})
target_link_libraries(golib goimg ${wxWidgets_LIBRARIES} RtAudio RtMidi ${PORTAUDIO_LIBRARIES} ${FFTW_LIBRARIES} ${WAVPACK_LIBRARIES})

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
   add_executable(${PROJECT_NAME} WIN32 GrandOrgue.cpp ${CMAKE_CURRENT_BINARY_DIR}/GrandOrgue.rc/GrandOrgue.rc)
   add_linker_option(${PROJECT_NAME} large-address-aware)

elseif (APPLE)

   configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resource/Info.plist.in ${CMAKE_CURRENT_BINARY_DIR}/temp.plist)
   configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resource/GrandOrgue.icns ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.app/Contents/Resources/${PROJECT_NAME}.icns COPYONLY)
   add_executable(${PROJECT_NAME} MACOSX_BUNDLE GrandOrgue.cpp)
   set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
   set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_BINARY_DIR}/temp.plist)

else ()

   add_executable(${PROJECT_NAME} GrandOrgue.cpp)

endif ()

target_link_libraries(${PROJECT_NAME} golib)

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
target_link_libraries(perftest golib)

add_custom_target(runperftest COMMAND perftest ${CMAKE_SOURCE_DIR}/tests DEPENDS perftest)
