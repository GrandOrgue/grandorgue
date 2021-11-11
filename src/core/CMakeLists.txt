# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

include(UsewxWidgets)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/go_defs.h.in ${CMAKE_CURRENT_BINARY_DIR}/go_defs.h/go_defs.h)
include_directories(${WAVPACK_INCLUDE_DIRS})
include_directories(${ZLIB_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/go_defs.h)

set(src
contrib/sha1.cpp
threading/GOCondition.cpp
threading/GOMutex.cpp
threading/GOWaitQueue.cpp
threading/GOThread.cpp
threading/threading_impl.cpp
GOSoundTouchWorkItem.cpp
GOArchive.cpp
GOArchiveCreator.cpp
GOArchiveFile.cpp
GOArchiveIndex.cpp
GOArchiveEntryFile.cpp
GOArchiveManager.cpp
GOArchiveReader.cpp
GOArchiveWriter.cpp
GOBitmap.cpp
GOCompress.cpp
GOConfigFileReader.cpp
GOConfigFileWriter.cpp
GOConfigReader.cpp
GOConfigReaderDB.cpp
GOConfigWriter.cpp
GODC.cpp
GODocumentBase.cpp
GOEvent.cpp
GOEventDistributor.cpp
GOFont.cpp
GOHash.cpp
GOInvalidFile.cpp
GOKeyConvert.cpp
GOKeyReceiverData.cpp
GOLoadThread.cpp
GOLog.cpp
GOLogWindow.cpp
GOMemoryPool.cpp
GOMidiEvent.cpp
GOMidiFileReader.cpp
GOMidiMap.cpp
GOMidiMerger.cpp
GOMidiOutputMerger.cpp
GOMidiPlayerContent.cpp
GOMidiReceiverBase.cpp
GOMidiReceiverData.cpp
GOMidiSenderData.cpp
GOMidiWXEvent.cpp
GOOrgan.cpp
GOOrganList.cpp
GOPath.cpp
GORodgers.cpp
GOSampleStatistic.cpp
GOSetting.cpp
GOSettingBool.cpp
GOSettingDirectory.cpp
GOSettingEnum.cpp
GOSettingFile.cpp
GOSettingFloat.cpp
GOSettingStore.cpp
GOSettingString.cpp
GOStandardFile.cpp
GOStdPath.cpp
GOTemperament.cpp
GOTemperamentCent.cpp
GOTemperamentList.cpp
GOTemperamentUser.cpp
GOTimer.cpp
GOUtil.cpp
GOView.cpp
GOWave.cpp
GOWavPack.cpp
GOWavPackWriter.cpp
MIDIList.cpp
OrganSelectDialog.cpp
SplashScreen.cpp
wxGaugeAudio.cpp
)

BUILD_LIBRARY(GrandOrgueCore ${src})
target_link_libraries(GrandOrgueCore GrandOrgueImages ${wxWidgets_LIBRARIES} ${WAVPACK_LIBRARIES} ${ZLIB_LIBRARIES})
link_directories(${wxWidgets_LIBRARY_DIRS} ${WAVPACK_LIBDIR} ${ZLIB_LIBDIR})
