/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2024-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundBufferPlanar.h"

#include <format>
#include <vector>

#include "sound/buffer/GOSoundBufferMutable.h"
#include "sound/buffer/GOSoundBufferPlanar.h"

const std::string GOTestSoundBufferPlanar::TEST_NAME
  = "GOTestSoundBufferPlanar";

void GOTestSoundBufferPlanar::TestConstructorAndBasicProperties() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 4;
  const unsigned nItems = nChannels * nFrames;

  std::vector<GOSoundBuffer::Item> data(nItems);
  fillWithSequential(data.data(), nItems, 1.0f);

  GOSoundBufferPlanar buffer(data.data(), nChannels, nFrames);

  AssertDimensions("ConstructorAndBasicProperties", buffer, nChannels, nFrames);

  GOAssert(
    buffer.GetData() == data.data(),
    std::format(
      "GetData() should return pointer to original data (expected: {}, got: "
      "{})",
      static_cast<const void *>(data.data()),
      static_cast<const void *>(buffer.GetData())));

  GOAssert(
    buffer.GetNItems() == nItems,
    std::format(
      "GetNItems() should return {} (got: {})", nItems, buffer.GetNItems()));

  GOAssert(
    buffer.GetNBytes() == nItems * sizeof(GOSoundBuffer::Item),
    std::format(
      "GetNBytes() should return {} (got: {})",
      nItems * sizeof(GOSoundBuffer::Item),
      buffer.GetNBytes()));

  AssertSequentialData("ConstructorAndBasicProperties", buffer, 1.0f);
}

void GOTestSoundBufferPlanar::TestGetChannelBuffer() {
  const unsigned nChannels = 3;
  const unsigned nFrames = 4;

  std::vector<GOSoundBuffer::Item> data(nChannels * nFrames);
  fillWithSequential(data.data(), data.size(), 0.0f);

  GOSoundBufferPlanar buffer(data.data(), nChannels, nFrames);

  for (unsigned channelI = 0; channelI < nChannels; ++channelI) {
    GOSoundBuffer channelBuffer = buffer.GetChannelBuffer(channelI);

    AssertDimensions(
      std::format("GetChannelBuffer({})", channelI), channelBuffer, 1, nFrames);

    GOAssert(
      channelBuffer.GetData() == data.data() + channelI * nFrames,
      std::format(
        "GetChannelBuffer({}) should point to channel start", channelI));

    AssertSequentialData(
      std::format("GetChannelBuffer({})", channelI),
      channelBuffer,
      static_cast<float>(channelI * nFrames));
  }
}

void GOTestSoundBufferPlanar::TestInterleaveTo() {
  const unsigned nChannels = 3;
  const unsigned nFrames = 4;

  // planarData[c * nFrames + f] = c * 100 + f
  std::vector<GOSoundBuffer::Item> planarData(nChannels * nFrames);
  for (unsigned channelI = 0; channelI < nChannels; ++channelI)
    for (unsigned frameI = 0; frameI < nFrames; ++frameI)
      planarData[channelI * nFrames + frameI]
        = static_cast<float>(channelI * 100 + frameI);

  GOSoundBufferPlanar planarBuffer(planarData.data(), nChannels, nFrames);

  std::vector<GOSoundBuffer::Item> interleavedData(nChannels * nFrames, -1.0f);
  GOSoundBufferMutable interleavedBuffer(
    interleavedData.data(), nChannels, nFrames);

  planarBuffer.InterleaveTo(interleavedBuffer);

  for (unsigned channelI = 0; channelI < nChannels; ++channelI)
    for (unsigned frameI = 0; frameI < nFrames; ++frameI)
      GOAssert(
        interleavedData[frameI * nChannels + channelI]
          == planarData[channelI * nFrames + frameI],
        std::format(
          "InterleaveTo: frame {} ch{} mismatch (expected: {}, got: {})",
          frameI,
          channelI,
          planarData[channelI * nFrames + frameI],
          interleavedData[frameI * nChannels + channelI]));
}

void GOTestSoundBufferPlanar::TestInvalidBuffer() {
  TestNullAndZeroDimensionInvalid<GOSoundBufferPlanar>("InvalidBuffer");
}

void GOTestSoundBufferPlanar::TestEdgeCases() {
  // 1-channel buffer: interleaved and planar layouts coincide
  const unsigned nFrames = 5;
  std::vector<GOSoundBuffer::Item> monoData(nFrames);
  fillWithSequential(monoData.data(), nFrames, 3.0f);

  GOSoundBufferPlanar monoBuffer(monoData.data(), 1, nFrames);
  AssertDimensions("EdgeCases mono", monoBuffer, 1, nFrames);
  AssertSequentialData("EdgeCases mono", monoBuffer, 3.0f);

  GOSoundBuffer channel0 = monoBuffer.GetChannelBuffer(0);
  GOAssert(
    channel0.GetData() == monoData.data(),
    "GetChannelBuffer(0) of a 1-channel buffer should point to the start");

  // 4-channel buffer, the case GOSoundFilter cannot handle today
  const unsigned nChannels4 = 4;
  std::vector<GOSoundBuffer::Item> data4(nChannels4 * nFrames);
  fillWithSequential(data4.data(), data4.size(), 0.0f);

  GOSoundBufferPlanar buffer4(data4.data(), nChannels4, nFrames);
  AssertDimensions("EdgeCases 4ch", buffer4, nChannels4, nFrames);

  for (unsigned channelI = 0; channelI < nChannels4; ++channelI)
    AssertDimensions(
      std::format("EdgeCases 4ch GetChannelBuffer({})", channelI),
      buffer4.GetChannelBuffer(channelI),
      1,
      nFrames);
}

void GOTestSoundBufferPlanar::run() {
  TestConstructorAndBasicProperties();
  TestGetChannelBuffer();
  TestInterleaveTo();
  TestInvalidBuffer();
  TestEdgeCases();
}
