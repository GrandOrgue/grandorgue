# GrandOrgue - free pipe organ simulator
# 
# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2016 GrandOrgue contributors (see AUTHORS)
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

# include wxWidgets
if (STATIC)
  set(wxWidgets_USE_STATIC ON)
else()
  set(wxWidgets_USE_STATIC OFF)
endif()

include(CMakeWxWidgetsInclude.txt)
include(${CMAKE_SOURCE_DIR}/cmake/AddLinkerOption.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/CopyWxTranslations.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/CopyDependencies.cmake)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/GrandOrgueDef.h.in ${CMAKE_CURRENT_BINARY_DIR}/GrandOrgueDef.h/GrandOrgueDef.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/GrandOrgueDef.h ${CMAKE_CURRENT_SOURCE_DIR}/contrib ${CMAKE_CURRENT_SOURCE_DIR}/resource)

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
GOrgueWave.cpp
GOrgueWavPack.cpp
GOrgueArchive.cpp
GOrgueArchiveEntryFile.cpp
GOrgueArchiveFile.cpp
GOrgueArchiveIndex.cpp
GOrgueArchiveManager.cpp
GOrgueArchiveReader.cpp
GOrgueAudioRecorder.cpp
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
GOrgueElementCreator.cpp
GOrgueEnclosure.cpp
GOrgueEvent.cpp
GOrgueEventDistributor.cpp
GOrgueFilename.cpp
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
GOrgueTimer.cpp
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
SplashScreen.cpp
wxGaugeAudio.cpp)

add_library(golib STATIC ${grandorgue_src})
target_link_libraries(golib goimg ${wxWidgets_LIBRARIES} RtAudio RtMidi ${PORTAUDIO_LIBRARIES} ${FFTW_LIBRARIES} ${WAVPACK_LIBRARIES})

if(APPLE)
  set(BINDIR "${PROJECT_NAME}.app/Contents/MacOS")
else()
  set(BINDIR "bin")
endif()

set(instpath "GrandOrgue${CMAKE_EXECUTABLE_SUFFIX}")

if (APPLE AND INSTALL_DEPEND STREQUAL "ON")
  set(NAME grandorgue)
  ADD_CUSTOM_COMMAND(OUTPUT "${CMAKE_BINARY_DIR}/${BINDIR}/${instpath}" COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${NAME}${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_BINARY_DIR}/${BINDIR}/${instpath}"  DEPENDS ${NAME})
  set(RUNTIME_DIR ${CMAKE_CURRENT_BINARY_DIR})
else()
  set(NAME GrandOrgue)
  set(RUNTIME_DIR ${CMAKE_BINARY_DIR}/${BINDIR})
endif()

if (WIN32)
   add_executable(${NAME} WIN32 GrandOrgue.cpp ${CMAKE_CURRENT_SOURCE_DIR}/resource/GrandOrgue.rc)
   add_linker_option(${NAME} large-address-aware)
else ()
   add_executable(${NAME} GrandOrgue.cpp)
endif ()

target_link_libraries(${NAME} golib)
set_target_properties(${NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${RUNTIME_DIR})

install(PROGRAMS "${RUNTIME_DIR}/${NAME}${CMAKE_EXECUTABLE_SUFFIX}" DESTINATION "${BINDIR}" RENAME "${instpath}")

if (INSTALL_DEPEND STREQUAL "ON")
  CopyWxTranslations()
  CopyDependencies("${BINDIR}/${instpath}" ${wxWidgets_LIBRARIES})
endif()

add_executable(perftest EXCLUDE_FROM_ALL perftest.cpp)
target_link_libraries(perftest golib)

add_custom_target(runperftest COMMAND perftest ${CMAKE_SOURCE_DIR}/tests DEPENDS perftest)
