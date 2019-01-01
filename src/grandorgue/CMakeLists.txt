# GrandOrgue - free pipe organ simulator
# 
# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2018 GrandOrgue contributors (see AUTHORS)
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

include(${CMAKE_SOURCE_DIR}/cmake/AddLinkerOption.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/CopyWxTranslations.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/CopyDependencies.cmake)

add_option(-mmmx)
add_option(-msse)
add_option(-msse2)
add_option(-msse3)

add_option(-mstackrealign)

include_directories(${CMAKE_BINARY_DIR}/src/core/GrandOrgueDef.h ${CMAKE_CURRENT_SOURCE_DIR}/contrib ${CMAKE_CURRENT_SOURCE_DIR}/resource ${CMAKE_SOURCE_DIR}/src/core)
include_directories(${RT_INCLUDE_DIRS})
include_directories(${PORTAUDIO_INCLUDE_DIRS})
include_directories(${FFTW_INCLUDE_DIRS})
include_directories(${wxWidgets_INCLUDE_DIRS})
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
GOrgueAudioRecorder.cpp
GOrgueCache.cpp
GOrgueCacheCleaner.cpp
GOrgueCacheWriter.cpp
GOrgueCombinationDefinition.cpp
GOrgueCombination.cpp
GOrgueBitmapCache.cpp
GOrgueButton.cpp
GOrgueCoupler.cpp
GOrgueDivisional.cpp
GOrgueDivisionalCoupler.cpp
GOrgueDummyPipe.cpp
GOrgueDrawStop.cpp
GOrgueElementCreator.cpp
GOrgueEnclosure.cpp
GOrgueFilename.cpp
GOrgueFrameGeneral.cpp
GOrgueGeneral.cpp
GOrgueKeyReceiver.cpp
GOrgueLabel.cpp
GOrgueManual.cpp
GOrgueMetronome.cpp
GOrgueMainWindowData.cpp
GOrgueMidi.cpp
GOrgueMidiListener.cpp
GOrgueMidiInPort.cpp
GOrgueMidiOutPort.cpp
GOrgueMidiPlayer.cpp
GOrgueMidiSender.cpp
GOrgueMidiReceiver.cpp
GOrgueMidiRecorder.cpp
GOrgueMidiRtInPort.cpp
GOrgueMidiRtOutPort.cpp
GOrgueModel.cpp
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
GOrgueSoundPort.cpp
GOrgueSoundPortaudioPort.cpp
GOrgueSoundRtPort.cpp
GOrgueSoundingPipe.cpp
GOrgueStop.cpp
GOrgueTremulant.cpp
GOrgueWindchest.cpp
GOrgueSwitch.cpp
GrandOrgueFile.cpp
GrandOrgueFrame.cpp
MIDIEventDialog.cpp
MIDIEventRecvDialog.cpp
MIDIEventSendDialog.cpp
MIDIEventKeyDialog.cpp
OrganDialog.cpp
GOrgueDocument.cpp
GOrguePanelView.cpp
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
   add_executable(GrandOrgue WIN32 GrandOrgue.cpp "${CMAKE_CURRENT_SOURCE_DIR}/resource/GrandOrgue.rc")
   add_linker_option(GrandOrgue large-address-aware)
   add_executable(GrandOrgueTool GrandOrgueTool.cpp "${CMAKE_CURRENT_SOURCE_DIR}/resource/GrandOrgue.rc")
   add_linker_option(GrandOrgueTool large-address-aware)
else ()
   add_executable(GrandOrgue GrandOrgue.cpp)
   add_executable(GrandOrgueTool GrandOrgueTool.cpp)
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
