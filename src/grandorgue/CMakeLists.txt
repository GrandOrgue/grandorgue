# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

include(${CMAKE_SOURCE_DIR}/cmake/AddLinkerOption.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/CopyWxTranslations.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/CopyDependencies.cmake)

add_option(-mmmx)
add_option(-msse)
add_option(-msse2)
add_option(-msse3)

add_option(-mstackrealign)

include_directories(${CMAKE_BINARY_DIR}/src/core/go_defs.h ${CMAKE_CURRENT_SOURCE_DIR}/contrib ${CMAKE_CURRENT_SOURCE_DIR}/resource ${CMAKE_SOURCE_DIR}/src/core)
include_directories(${RT_INCLUDE_DIRS})
include_directories(${PORTAUDIO_INCLUDE_DIRS})
include_directories(${FFTW_INCLUDE_DIRS})
include_directories(${wxWidgets_INCLUDE_DIRS})
include_directories(${JACK_INCLUDE_DIRS})
include(UsewxWidgets)

set(grandorgue_src
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
GOGUIRecorderPanel.cpp
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
GOSoundReleaseWorkItem.cpp
GOSoundSamplerPool.cpp
GOSoundScheduler.cpp
GOSoundThread.cpp
GOSoundTremulantWorkItem.cpp
GOSoundWindchestWorkItem.cpp
GOAudioRecorder.cpp
GOCache.cpp
GOCacheCleaner.cpp
GOCacheWriter.cpp
GOCombinationDefinition.cpp
GOCombination.cpp
GOBitmapCache.cpp
GOButton.cpp
GOCoupler.cpp
GODivisional.cpp
GODivisionalCoupler.cpp
GODummyPipe.cpp
GODrawStop.cpp
GOElementCreator.cpp
GOEnclosure.cpp
GOFilename.cpp
GOFrameGeneral.cpp
GOGeneral.cpp
GOKeyReceiver.cpp
GOLabel.cpp
GOManual.cpp
GOMetronome.cpp
GOMainWindowData.cpp
GOMidi.cpp
GOMidiListener.cpp
GOMidiInPort.cpp
GOMidiOutPort.cpp
GOMidiPlayer.cpp
GOMidiSender.cpp
GOMidiReceiver.cpp
GOMidiRecorder.cpp
GOMidiRtFactory.cpp
GOMidiRtInPort.cpp
GOMidiRtOutPort.cpp
GOModel.cpp
GOPipe.cpp
GOPipeConfig.cpp
GOPipeConfigNode.cpp
GOPipeConfigTreeNode.cpp
GOPiston.cpp
GOProgressDialog.cpp
GOProperties.cpp
GOPushbutton.cpp
GORank.cpp
GOReferencePipe.cpp
GOReleaseAlignTable.cpp
GOSetter.cpp
GOSetterButton.cpp
GOSettings.cpp
GOSound.cpp
GOSoundJackPort.cpp
GOSoundPort.cpp
GOSoundPortaudioPort.cpp
GOSoundPortsConfig.cpp
GOSoundRtPort.cpp
GOSoundingPipe.cpp
GOStop.cpp
GOTremulant.cpp
GOWindchest.cpp
GOSwitch.cpp
GODefinitionFile.cpp
GOFrame.cpp
MIDIEventDialog.cpp
MIDIEventRecvDialog.cpp
MIDIEventSendDialog.cpp
MIDIEventKeyDialog.cpp
OrganDialog.cpp
GODocument.cpp
GOPanelView.cpp
SettingsArchives.cpp
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
)

add_library(golib STATIC ${grandorgue_src})
set(go_libs ${wxWidgets_LIBRARIES} ${RT_LIBRARIES} ${PORTAUDIO_LIBRARIES} ${FFTW_LIBRARIES})
set(go_libdir ${wxWidgets_LIBRARY_DIRS} ${RT_LIBDIR} ${PORTAUDIO_LIBDIR} ${FFTW_LIBDIR})
target_link_libraries(golib GrandOrgueImages GrandOrgueCore ${go_libs})
link_directories(${go_libdir})

if (WIN32)
   set_source_files_properties("${RESOURCEDIR}/GrandOrgue.rc" PROPERTIES GENERATED "YES")
   add_executable(GrandOrgue WIN32 GOApp.cpp "${RESOURCEDIR}/GrandOrgue.rc")
   add_dependencies(GrandOrgue resources) # GrandOrgue.rc and GrandOrgue.manifest & GOIcon.ico referenced from GrandOrgue.rc
   add_linker_option(GrandOrgue large-address-aware)
   add_executable(GrandOrgueTool GOTool.cpp "${RESOURCEDIR}/GrandOrgue.rc")
   add_dependencies(GrandOrgueTool resources) # GrandOrgue.rc and GrandOrgue.manifest & GOIcon.ico referenced from GrandOrgue.rc
   add_linker_option(GrandOrgueTool large-address-aware)
else ()
   add_executable(GrandOrgue GOApp.cpp)
   add_executable(GrandOrgueTool GOTool.cpp)
endif ()

BUILD_EXECUTABLE(GrandOrgue)
target_link_libraries(GrandOrgue golib)
BUILD_EXECUTABLE(GrandOrgueTool)
target_link_libraries(GrandOrgueTool golib)

if (INSTALL_DEPEND STREQUAL "ON")
  CopyWxTranslations()
  CopyDependencies(GrandOrgue "${BININSTDIR}/GrandOrgue${CMAKE_EXECUTABLE_SUFFIX}" ${go_libs} ${go_libdir})
endif()

add_executable(perftest EXCLUDE_FROM_ALL perftest.cpp)
target_link_libraries(perftest golib)

add_custom_target(runperftest COMMAND perftest "${CMAKE_SOURCE_DIR}/tests" DEPENDS perftest)

if (GO_USE_JACK STREQUAL "ON")
   add_definitions(-DGO_USE_JACK)
   target_link_libraries(GrandOrgue PkgConfig::JACK)
endif ()
