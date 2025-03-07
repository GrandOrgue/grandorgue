# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

include(${CMAKE_SOURCE_DIR}/cmake/AddLinkerOption.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/CopyWxTranslations.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/CopyDependencies.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/AddCpuOptions.cmake)

find_package(wxWidgets REQUIRED html net adv core base)
include(${wxWidgets_USE_FILE})
if (APPLE)
  # /usr/local/share/cmake/yaml-cpp/yaml-cpp-config.cmake sets
  # empty YAML_CPP_INCLUDE_DIR and YAML_CPP_LIBRARIES, so we use pkg_config
  pkg_check_modules(YAML_CPP REQUIRED yaml-cpp)
else()
  # but pkg_config does not support static linking for yaml-cpp, so use cmake config
  ## at first, try to find static package
  set(CMAKE_FIND_DEBUG_MODE TRUE)
  find_package(yaml-cpp REQUIRED)
  set(CMAKE_FIND_DEBUG_MODE FALSE)
  message("yaml-cpp_CONFIG=${yaml-cpp_CONFIG}")
  include("${yaml-cpp_CONFIG}")
  # the variables are different in both methods
  set(YAML_CPP_INCLUDE_DIRS "${YAML_CPP_INCLUDE_DIR}")
endif()

message("YAML_CPP_INCLUDE_DIRS=${YAML_CPP_INCLUDE_DIRS}")
message("YAML_CPP_LIBRARIES=${YAML_CPP_LIBRARIES}")

include_directories(${CMAKE_BINARY_DIR}/src/core/go_defs.h ${CMAKE_CURRENT_SOURCE_DIR}/resource ${CMAKE_SOURCE_DIR}/src/core)
include_directories(${RT_INCLUDE_DIRS})
include_directories(${PORTAUDIO_INCLUDE_DIRS})
include_directories(${ZITACONVOLVER_INCLUDE_DIRS})
include_directories(${FFTW_INCLUDE_DIRS})
include_directories(${wxWidgets_INCLUDE_DIRS})
include_directories(${JACK_INCLUDE_DIRS})
include_directories(${YAML_CPP_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(grandorgue_src
combinations/control/GOCombinationButtonSet.cpp
combinations/control/GOCombinationControllerProxy.cpp
combinations/control/GODivisionalButtonControl.cpp
combinations/control/GOGeneralButtonControl.cpp
combinations/model/GOCombination.cpp
combinations/model/GOCombinationDefinition.cpp
combinations/model/GODivisionalCombination.cpp
combinations/model/GOGeneralCombination.cpp
combinations/GODivisionalSetter.cpp
combinations/GOSetter.cpp
config/GOAudioDeviceConfig.cpp
config/GOAudioDeviceNode.cpp
config/GOConfig.cpp
config/GODeviceNamePattern.cpp
config/GOMidiDeviceConfig.cpp
config/GOMidiDeviceConfigList.cpp
config/GOPortsConfig.cpp
config/GOPortFactory.cpp
control/GOButtonControl.cpp
control/GOCallbackButtonControl.cpp
control/GOElementCreator.cpp
control/GOEventDistributor.cpp
control/GOLabelControl.cpp
control/GOPistonControl.cpp
document-base/GODocumentBase.cpp
document-base/GOView.cpp
gui/dialogs/common/GODialogCloser.cpp
gui/dialogs/common/GODialogSizeSet.cpp
gui/dialogs/common/GODialogTab.cpp
gui/dialogs/common/GOSimpleDialog.cpp
gui/dialogs/common/GOTabbedDialog.cpp
gui/dialogs/midi-event/GOMidiEventDialog.cpp
gui/dialogs/midi-event/GOMidiEventKeyTab.cpp
gui/dialogs/midi-event/GOMidiEventRecvTab.cpp
gui/dialogs/midi-event/GOMidiEventSendTab.cpp
gui/dialogs/organ-settings/GOOrganSettingsTab.cpp
gui/dialogs/organ-settings/GOOrganSettingsDialog.cpp
gui/dialogs/organ-settings/GOOrganSettingsEnclosuresTab.cpp
gui/dialogs/organ-settings/GOOrganSettingsPipesTab.cpp
gui/dialogs/settings/GOSettingsAudio.cpp
gui/dialogs/settings/GOSettingsDeviceMatchDialog.cpp
gui/dialogs/settings/GOSettingsDialog.cpp
gui/dialogs/settings/GOSettingsMidiDeviceList.cpp
gui/dialogs/settings/GOSettingsMidiDevices.cpp
gui/dialogs/settings/GOSettingsMidiMessage.cpp
gui/dialogs/settings/GOSettingsPaths.cpp
gui/dialogs/settings/GOSettingsPorts.cpp
gui/dialogs/settings/GOSettingsOptions.cpp
gui/dialogs/settings/GOSettingsOrgans.cpp
gui/dialogs/settings/GOSettingsReverb.cpp
gui/dialogs/settings/GOSettingsTemperaments.cpp
gui/dialogs/GOMidiObjectsDialog.cpp
gui/dialogs/GONewReleaseDialog.cpp
gui/dialogs/GOProgressDialog.cpp
gui/dialogs/GOPropertiesDialog.cpp
gui/dialogs/GOSelectOrganDialog.cpp
gui/dialogs/GOSplash.cpp
gui/frames/GOFrame.cpp
gui/frames/GOLogWindow.cpp
gui/frames/GOMainWindowData.cpp
gui/frames/GOStopsWindow.cpp
gui/panels/primitives/GOBitmap.cpp
gui/panels/primitives/GODC.cpp
gui/panels/primitives/GOFont.cpp
gui/panels/GOGUIBankedGeneralsPanel.cpp
gui/panels/GOGUIButton.cpp
gui/panels/GOGUICrescendoPanel.cpp
gui/panels/GOGUIControl.cpp
gui/panels/GOGUICouplerPanel.cpp
gui/panels/GOGUIDisplayMetrics.cpp
gui/panels/GOGUIDivisionalsPanel.cpp
gui/panels/GOGUIEnclosure.cpp
gui/panels/GOGUIFloatingPanel.cpp
gui/panels/GOGUIHW1Background.cpp
gui/panels/GOGUIHW1DisplayMetrics.cpp
gui/panels/GOGUIImage.cpp
gui/panels/GOGUILayoutEngine.cpp
gui/panels/GOGUIPanelView.cpp
gui/panels/GOGUISetterDisplayMetrics.cpp
gui/panels/GOGUILabel.cpp
gui/panels/GOGUIManual.cpp
gui/panels/GOGUIManualBackground.cpp
gui/panels/GOGUIMasterPanel.cpp
gui/panels/GOGUIMetronomePanel.cpp
gui/panels/GOGUIPanel.cpp
gui/panels/GOGUIPanelWidget.cpp
gui/panels/GOGUIRecorderPanel.cpp
gui/panels/GOGUISequencerPanel.cpp
gui/size/GOSizeKeeper.cpp
gui/wxcontrols/GOAudioGauge.cpp
gui/wxcontrols/GOGrid.cpp
gui/wxcontrols/GORightVisiblePicker.cpp
gui/wxcontrols/go_gui_utils.cpp
help/GOHelpController.cpp
help/GOHelpRequestor.cpp
loader/GOFileStore.cpp
loader/GOLoaderFilename.cpp
loader/GOLoadThread.cpp
loader/GOLoadWorker.cpp
loader/cache/GOCache.cpp
loader/cache/GOCacheCleaner.cpp
loader/cache/GOCacheWriter.cpp
midi/dialog-creator/GOMidiDialogCreatorProxy.cpp
midi/objects/GOMidiObject.cpp
midi/objects/GOMidiObjectWithDivision.cpp
midi/objects/GOMidiObjectWithShortcut.cpp
midi/objects/GOMidiReceivingSendingObject.cpp
midi/objects/GOMidiSendingObject.cpp
midi/ports/GOMidiInPort.cpp
midi/ports/GOMidiOutPort.cpp
midi/ports/GOMidiPort.cpp
midi/ports/GOMidiPortFactory.cpp
midi/ports/GOMidiRtPortFactory.cpp
midi/ports/GOMidiRtInPort.cpp
midi/ports/GOMidiRtOutPort.cpp
midi/GOMidi.cpp
midi/GOMidiInputMerger.cpp
midi/GOMidiListener.cpp
midi/GOMidiOutputMerger.cpp
midi/GOMidiPlayer.cpp
midi/GOMidiPlayerContent.cpp
midi/GOMidiSendProxy.cpp
midi/GOMidiSender.cpp
midi/GOMidiShortcutPattern.cpp
midi/GOMidiShortcutReceiver.cpp
midi/GOMidiReceiver.cpp
midi/GOMidiRecorder.cpp
model/pipe-config/GOPipeConfig.cpp
model/pipe-config/GOPipeConfigNode.cpp
model/pipe-config/GOPipeConfigTreeNode.cpp
model/GOCacheObject.cpp
model/GOCoupler.cpp
model/GODivisionalCoupler.cpp
model/GODrawstop.cpp
model/GOEnclosure.cpp
model/GOEventHandlerList.cpp
model/GOManual.cpp
model/GOOrganModel.cpp
model/GOPipe.cpp
model/GORank.cpp
model/GOReferencePipe.cpp
model/GOReferencingObject.cpp
model/GOSoundingPipe.cpp
model/GOStop.cpp
model/GOSwitch.cpp
model/GOTremulant.cpp
model/GOWindchest.cpp
modification/GOModificationProxy.cpp
sound/ports/GOSoundJackPort.cpp
sound/ports/GOSoundPort.cpp
sound/ports/GOSoundPortFactory.cpp
sound/ports/GOSoundPortaudioPort.cpp
sound/ports/GOSoundRtPort.cpp
sound/scheduler/GOSoundGroupTask.cpp
sound/scheduler/GOSoundOutputTask.cpp
sound/scheduler/GOSoundReleaseTask.cpp
sound/scheduler/GOSoundScheduler.cpp
sound/scheduler/GOSoundThread.cpp
sound/scheduler/GOSoundTouchTask.cpp
sound/scheduler/GOSoundTremulantTask.cpp
sound/scheduler/GOSoundWindchestTask.cpp
sound/GOSoundAudioSection.cpp
sound/GOSoundDevInfo.cpp
sound/GOSoundEngine.cpp
sound/GOSoundFader.cpp
sound/GOSoundProvider.cpp
sound/GOSoundProviderSynthedTrem.cpp
sound/GOSoundProviderWave.cpp
sound/GOSoundRecorder.cpp
sound/GOSoundReleaseAlignTable.cpp
sound/GOSoundReverb.cpp
sound/GOSoundReverbEngine.cpp
sound/GOSoundReverbPartition.cpp
sound/GOSoundResample.cpp
sound/GOSoundSamplerPool.cpp
sound/GOSoundStateHandler.cpp
sound/GOSoundStream.cpp
sound/GOSound.cpp
sound/GOSoundFilter.cpp
sound/GOSoundToneBalanceFilter.cpp
updater/GOUpdateChecker.cpp
yaml/GOSaveableToYaml.cpp
yaml/go-wx-yaml.cpp
GOAudioRecorder.cpp
GOBitmapCache.cpp
GOEvent.cpp
GOLog.cpp
GOMetronome.cpp
GOOrganController.cpp
GODocument.cpp
GOVirtualCouplerController.cpp
)

if (USE_INTERNAL_ZITACONVOLVER)
  list(APPEND grandorgue_src ${CMAKE_SOURCE_DIR}/submodules/ZitaConvolver/source/zita-convolver.cc)
endif()

add_library(golib STATIC ${grandorgue_src})
set(go_libs ${wxWidgets_LIBRARIES} ${YAML_CPP_LIBRARIES} ${RT_LIBRARIES} ${PORTAUDIO_LIBRARIES} ${FFTW_LIBRARIES} ${ZITACONVOLVER_LIBRARIES} CURL::libcurl)
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

if (GO_USE_JACK)
   add_definitions(-DGO_USE_JACK)
   target_link_libraries(golib PkgConfig::JACK)
endif ()
