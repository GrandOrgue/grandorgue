# GrandOrgue - free pipe organ simulator
# 
# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2017 GrandOrgue contributors (see AUTHORS)
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

include(UsewxWidgets)
include_directories(${WAVPACK_INCLUDE_DIRS})

set(src
contrib/sha1.cpp
GOrgueBitmap.cpp
GOrgueConfigFileReader.cpp
GOrgueConfigFileWriter.cpp
GOrgueConfigReader.cpp
GOrgueConfigReaderDB.cpp
GOrgueConfigWriter.cpp
GOrgueDC.cpp
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
GOrgueMidiPlayerContent.cpp
GOrgueMidiSenderData.cpp
GOrgueMidiReceiverData.cpp
GOrgueMidiWXEvent.cpp
GOrguePath.cpp
GOrgueSampleStatistic.cpp
GOrgueSetting.cpp
GOrgueSettingBool.cpp
GOrgueSettingDirectory.cpp
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
GOrgueThread.cpp
GOrgueTimer.cpp
GOrgueUtil.cpp
GOrgueWave.cpp
GOrgueWavPack.cpp
)

BUILD_LIBRARY(GrandOrgueCore ${src})
target_link_libraries(GrandOrgueCore ${wxWidgets_LIBRARIES} ${WAVPACK_LIBRARIES})
link_directories(${wxWidgets_LIBRARY_DIRS} ${WAVPACK_LIBDIR})
