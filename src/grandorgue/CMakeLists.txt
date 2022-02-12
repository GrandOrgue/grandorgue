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
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include(UsewxWidgets)

set(grandorgue_src
config/GOConfig.cpp
config/GOMidiDeviceConfig.cpp
config/GOMidiDeviceConfigList.cpp
config/GOPortsConfig.cpp
config/GOPortFactory.cpp
contrib/zita-convolver.cpp
dialogs/midi-event/GOMidiEventDialog.cpp
dialogs/midi-event/GOMidiEventKeyTab.cpp
dialogs/midi-event/GOMidiEventRecvTab.cpp
dialogs/midi-event/GOMidiEventSendTab.cpp
dialogs/settings/GOSettingsAudio.cpp
dialogs/settings/GOSettingsDialog.cpp
dialogs/settings/GOSettingsMidiDeviceList.cpp
dialogs/settings/GOSettingsMidiDevices.cpp
dialogs/settings/GOSettingsMidiMatchDialog.cpp
dialogs/settings/GOSettingsMidiMessage.cpp
dialogs/settings/GOSettingsPaths.cpp
dialogs/settings/GOSettingsPorts.cpp
dialogs/settings/GOSettingsOptions.cpp
dialogs/settings/GOSettingsOrgans.cpp
dialogs/settings/GOSettingsReverb.cpp
dialogs/settings/GOSettingsTemperaments.cpp
dialogs/GOOrganDialog.cpp
dialogs/GOProgressDialog.cpp
dialogs/GOSplash.cpp
gui/GOGUIBankedGeneralsPanel.cpp
gui/GOGUIButton.cpp
gui/GOGUICrescendoPanel.cpp
gui/GOGUIControl.cpp
gui/GOGUICouplerPanel.cpp
gui/GOGUIDisplayMetrics.cpp
gui/GOGUIDivisionalsPanel.cpp
gui/GOGUILayoutEngine.cpp
gui/GOGUIHW1DisplayMetrics.cpp
gui/GOGUISetterDisplayMetrics.cpp
gui/GOGUIEnclosure.cpp
gui/GOGUIFloatingPanel.cpp
gui/GOGUIHW1Background.cpp
gui/GOGUIImage.cpp
gui/GOGUILabel.cpp
gui/GOGUIManual.cpp
gui/GOGUIManualBackground.cpp
gui/GOGUIMasterPanel.cpp
gui/GOGUIMetronomePanel.cpp
gui/GOGUIPanel.cpp
gui/GOGUIPanelWidget.cpp
gui/GOGUIRecorderPanel.cpp
gui/GOGUISequencerPanel.cpp
midi/ports/GOMidiInPort.cpp
midi/ports/GOMidiOutPort.cpp
midi/ports/GOMidiPort.cpp
midi/ports/GOMidiPortFactory.cpp
midi/ports/GOMidiRtPortFactory.cpp
midi/ports/GOMidiRtInPort.cpp
midi/ports/GOMidiRtOutPort.cpp
midi/GOMidi.cpp
midi/GOMidiListener.cpp
midi/GOMidiInputMerger.cpp
midi/GOMidiOutputMerger.cpp
midi/GOMidiPlayer.cpp
midi/GOMidiPlayerContent.cpp
midi/GOMidiSender.cpp
midi/GOMidiReceiver.cpp
midi/GOMidiRecorder.cpp
sound/ports/GOSoundJackPort.cpp
sound/ports/GOSoundPort.cpp
sound/ports/GOSoundPortFactory.cpp
sound/ports/GOSoundPortaudioPort.cpp
sound/ports/GOSoundRtPort.cpp
sound/scheduler/GOSoundGroupWorkItem.cpp
sound/scheduler/GOSoundOutputWorkItem.cpp
sound/scheduler/GOSoundReleaseWorkItem.cpp
sound/scheduler/GOSoundScheduler.cpp
sound/scheduler/GOSoundThread.cpp
sound/scheduler/GOSoundTouchWorkItem.cpp
sound/scheduler/GOSoundTremulantWorkItem.cpp
sound/scheduler/GOSoundWindchestWorkItem.cpp
sound/GOSoundAudioSection.cpp
sound/GOSoundEngine.cpp
sound/GOSoundProvider.cpp
sound/GOSoundProviderSynthedTrem.cpp
sound/GOSoundProviderWave.cpp
sound/GOSoundRecorder.cpp
sound/GOSoundReverb.cpp
sound/GOSoundReverbEngine.cpp
sound/GOSoundReverbPartition.cpp
sound/GOSoundResample.cpp
sound/GOSoundSamplerPool.cpp
sound/GOSound.cpp
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
GOModel.cpp
GOPipe.cpp
GOPipeConfig.cpp
GOPipeConfigNode.cpp
GOPipeConfigTreeNode.cpp
GOPiston.cpp
GOProperties.cpp
GOPushbutton.cpp
GORank.cpp
GOReferencePipe.cpp
GOReleaseAlignTable.cpp
GOSetter.cpp
GOSetterButton.cpp
GOSoundingPipe.cpp
GOStop.cpp
GOTremulant.cpp
GOWindchest.cpp
GOSwitch.cpp
GODefinitionFile.cpp
GOFrame.cpp
GODocument.cpp
GOPanelView.cpp
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
else ()
   add_executable(GrandOrgue GOApp.cpp)
endif ()

BUILD_EXECUTABLE(GrandOrgue)
target_link_libraries(GrandOrgue golib)

if (INSTALL_DEPEND STREQUAL "ON")
  CopyWxTranslations()
  CopyDependencies(GrandOrgue "${BININSTDIR}/GrandOrgue${CMAKE_EXECUTABLE_SUFFIX}" ${go_libs} ${go_libdir})
endif()

if (GO_USE_JACK STREQUAL "ON")
   add_definitions(-DGO_USE_JACK)
   target_link_libraries(GrandOrgue PkgConfig::JACK)
endif ()
