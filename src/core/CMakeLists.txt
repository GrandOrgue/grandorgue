# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

include(UsewxWidgets)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/go_defs.h.in ${CMAKE_CURRENT_BINARY_DIR}/go_defs.h/go_defs.h)
include_directories(${WAVPACK_INCLUDE_DIRS})
include_directories(${ZLIB_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/go_defs.h)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(src
archive/GOArchive.cpp
archive/GOArchiveCreator.cpp
archive/GOArchiveFile.cpp
archive/GOArchiveIndex.cpp
archive/GOArchiveEntryFile.cpp
archive/GOArchiveManager.cpp
archive/GOArchiveReader.cpp
archive/GOArchiveWriter.cpp
config/GOConfigFileReader.cpp
config/GOConfigFileWriter.cpp
config/GOConfigReader.cpp
config/GOConfigReaderDB.cpp
config/GOConfigWriter.cpp
contrib/sha1.cpp
files/GOInvalidFile.cpp
files/GOStandardFile.cpp
files/GOStdFileName.cpp
midi/GOMidiEvent.cpp
midi/GOMidiFileReader.cpp
midi/GOMidiMap.cpp
midi/GOMidiEventPattern.cpp
midi/GOMidiReceiverBase.cpp
midi/GOMidiReceiverEventPattern.cpp
midi/GOMidiSenderEventPattern.cpp
midi/GOMidiWXEvent.cpp
settings/GOSetting.cpp
settings/GOSettingBool.cpp
settings/GOSettingDirectory.cpp
settings/GOSettingEnum.cpp
settings/GOSettingFile.cpp
settings/GOSettingFloat.cpp
settings/GOSettingStore.cpp
settings/GOSettingString.cpp
threading/GOCondition.cpp
threading/GOMutex.cpp
threading/GOMutexLocker.cpp
threading/GOThread.cpp
threading/threading_impl.cpp
temperaments/GOTemperament.cpp
temperaments/GOTemperamentCent.cpp
temperaments/GOTemperamentList.cpp
temperaments/GOTemperamentUser.cpp
GOCompress.cpp
GOHash.cpp
GOKeyConvert.cpp
GOLogicalColour.cpp
GOMemoryPool.cpp
GOOrgan.cpp
GOOrganList.cpp
GOPath.cpp
GORodgers.cpp
GOSampleStatistic.cpp
GOStdPath.cpp
GOTimer.cpp
GOUtil.cpp
GOWave.cpp
GOWavPack.cpp
GOWavPackWriter.cpp
)

BUILD_LIBRARY(GrandOrgueCore ${src})
target_link_libraries(GrandOrgueCore ${wxWidgets_LIBRARIES} ${WAVPACK_LIBRARIES} ${ZLIB_LIBRARIES})
link_directories(${wxWidgets_LIBRARY_DIRS} ${WAVPACK_LIBDIR} ${ZLIB_LIBDIR})
