# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

include(UsewxWidgets)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/GrandOrgueDef.h.in ${CMAKE_CURRENT_BINARY_DIR}/GrandOrgueDef.h/GrandOrgueDef.h)
include_directories(${WAVPACK_INCLUDE_DIRS})
include_directories(${ZLIB_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/GrandOrgueDef.h)

set(src
contrib/sha1.cpp
threading/GOCondition.cpp
threading/GOMutex.cpp
threading/GOWaitQueue.cpp
threading/GOrgueThread.cpp
threading/threading_impl.cpp
GOSoundTouchWorkItem.cpp
GOrgueArchive.cpp
GOrgueArchiveCreator.cpp
GOrgueArchiveFile.cpp
GOrgueArchiveIndex.cpp
GOrgueArchiveEntryFile.cpp
GOrgueArchiveManager.cpp
GOrgueArchiveReader.cpp
GOrgueArchiveWriter.cpp
GOrgueBitmap.cpp
GOrgueCompress.cpp
GOrgueConfigFileReader.cpp
GOrgueConfigFileWriter.cpp
GOrgueConfigReader.cpp
GOrgueConfigReaderDB.cpp
GOrgueConfigWriter.cpp
GOrgueDC.cpp
GOrgueDocumentBase.cpp
GOrgueEvent.cpp
GOrgueEventDistributor.cpp
GOrgueFont.cpp
GOrgueHash.cpp
GOrgueInvalidFile.cpp
GOrgueKeyConvert.cpp
GOrgueKeyReceiverData.cpp
GOrgueLoadThread.cpp
GOrgueLog.cpp
GOrgueLogWindow.cpp
GOrgueMemoryPool.cpp
GOrgueMidiEvent.cpp
GOrgueMidiFileReader.cpp
GOrgueMidiMap.cpp
GOrgueMidiMerger.cpp
GOrgueMidiOutputMerger.cpp
GOrgueMidiPlayerContent.cpp
GOrgueMidiReceiverBase.cpp
GOrgueMidiReceiverData.cpp
GOrgueMidiSenderData.cpp
GOrgueMidiWXEvent.cpp
GOrgueOrgan.cpp
GOrgueOrganList.cpp
GOrguePath.cpp
GOrgueRodgers.cpp
GOrgueSampleStatistic.cpp
GOrgueSetting.cpp
GOrgueSettingBool.cpp
GOrgueSettingDirectory.cpp
GOrgueSettingEnum.cpp
GOrgueSettingFile.cpp
GOrgueSettingFloat.cpp
GOrgueSettingStore.cpp
GOrgueSettingString.cpp
GOrgueStandardFile.cpp
GOrgueStdPath.cpp
GOrgueTemperament.cpp
GOrgueTemperamentCent.cpp
GOrgueTemperamentList.cpp
GOrgueTemperamentUser.cpp
GOrgueTimer.cpp
GOrgueUtil.cpp
GOrgueView.cpp
GOrgueWave.cpp
GOrgueWavPack.cpp
GOrgueWavPackWriter.cpp
MIDIList.cpp
OrganSelectDialog.cpp
SplashScreen.cpp
wxGaugeAudio.cpp
)

BUILD_LIBRARY(GrandOrgueCore ${src})
target_link_libraries(GrandOrgueCore GrandOrgueImages ${wxWidgets_LIBRARIES} ${WAVPACK_LIBRARIES} ${ZLIB_LIBRARIES})
link_directories(${wxWidgets_LIBRARY_DIRS} ${WAVPACK_LIBDIR} ${ZLIB_LIBDIR})
