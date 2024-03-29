# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

include(${CMAKE_SOURCE_DIR}/cmake/AddLinkerOption.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/CopyWxTranslations.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/CopyDependencies.cmake)

add_option(-mmmx)
add_option(-msse)
add_option(-msse2)
add_option(-msse3)

add_option(-mstackrealign)

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
config/GOConfig.cpp
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
dialogs/common/GODialogCloser.cpp
dialogs/common/GODialogSizeSet.cpp
dialogs/common/GODialogTab.cpp
dialogs/common/GOSimpleDialog.cpp
dialogs/common/GOTabbedDialog.cpp
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
dialogs/GOMidiListDialog.cpp
dialogs/GONewReleaseDialog.cpp
dialogs/GOOrganSettingsDialog.cpp
dialogs/GOProgressDialog.cpp
dialogs/GOSelectOrganDialog.cpp
dialogs/GOSplash.cpp
document-base/GODocumentBase.cpp
document-base/GOView.cpp
gui/primitives/GOBitmap.cpp
gui/primitives/GODC.cpp
gui/primitives/GOFont.cpp
gui/primitives/go_gui_utils.cpp
gui/GOGUIBankedGeneralsPanel.cpp
gui/GOGUIButton.cpp
gui/GOGUICrescendoPanel.cpp
gui/GOGUIControl.cpp
gui/GOGUICouplerPanel.cpp
gui/GOGUIDisplayMetrics.cpp
gui/GOGUIDivisionalsPanel.cpp
gui/GOGUIEnclosure.cpp
gui/GOGUIFloatingPanel.cpp
gui/GOGUIHW1Background.cpp
gui/GOGUIHW1DisplayMetrics.cpp
gui/GOGUIImage.cpp
gui/GOGUILayoutEngine.cpp
gui/GOGUIPanelView.cpp
gui/GOGUISetterDisplayMetrics.cpp
gui/GOGUILabel.cpp
gui/GOGUIManual.cpp
gui/GOGUIManualBackground.cpp
gui/GOGUIMasterPanel.cpp
gui/GOGUIMetronomePanel.cpp
gui/GOGUIPanel.cpp
gui/GOGUIPanelWidget.cpp
gui/GOGUIRecorderPanel.cpp
gui/GOGUISequencerPanel.cpp
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
midi/ports/GOMidiInPort.cpp
midi/ports/GOMidiOutPort.cpp
midi/ports/GOMidiPort.cpp
midi/ports/GOMidiPortFactory.cpp
midi/ports/GOMidiRtPortFactory.cpp
midi/ports/GOMidiRtInPort.cpp
midi/ports/GOMidiRtOutPort.cpp
midi/GOMidi.cpp
midi/GOMidiConfigurator.cpp
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
model/GODrawStop.cpp
model/GOEnclosure.cpp
model/GOEventHandlerList.cpp
model/GOManual.cpp
model/GOOrganModel.cpp
model/GOPipe.cpp
model/GORank.cpp
model/GOReferencePipe.cpp
model/GOSoundingPipe.cpp
model/GOStop.cpp
model/GOSwitch.cpp
model/GOTremulant.cpp
model/GOWindchest.cpp
modification/GOModificationProxy.cpp
size/GOSizeKeeper.cpp
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
sound/GOSoundReleaseAlignTable.cpp
sound/GOSoundReverb.cpp
sound/GOSoundReverbEngine.cpp
sound/GOSoundReverbPartition.cpp
sound/GOSoundResample.cpp
sound/GOSoundSamplerPool.cpp
sound/GOSoundStateHandler.cpp
sound/GOSound.cpp
updater/GOUpdateChecker.cpp
yaml/GOSaveableToYaml.cpp
yaml/go-wx-yaml.cpp
wxcontrols/GOAudioGauge.cpp
wxcontrols/GOGrid.cpp
wxcontrols/GORightVisiblePicker.cpp
GOAudioRecorder.cpp
GOBitmapCache.cpp
GOEvent.cpp
GOLog.cpp
GOLogWindow.cpp
GOMetronome.cpp
GOMainWindowData.cpp
GOOrganController.cpp
GOProperties.cpp
GOFrame.cpp
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
