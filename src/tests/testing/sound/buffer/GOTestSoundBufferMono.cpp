/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2024-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundBufferMono.h"

#include <cstring>
#include <format>
#include <vector>

#include "sound/buffer/GOSoundBufferMono.h"
#include "sound/buffer/GOSoundBufferMutableMono.h"
#include "sound/buffer/GOSoundBufferPlanar.h"

#include "GOTestScope.h"

const std::string GOTestSoundBufferMono::TEST_NAME = "GOTestSoundBufferMono";

void GOTestSoundBufferMono::TestConstructorAndBasicProperties() {
  const unsigned nFrames = 5;

  std::vector<GOSoundBuffer::Item> data(nFrames);

  fillWithSequential(data.data(), nFrames, 1.0f);

  GOSoundBufferMono buffer(data.data(), nFrames);

  AssertDimensions("ConstructorAndBasicProperties", buffer, 1, nFrames);

  GOAssert(
    buffer.GetData() == data.data(),
    std::format(
      "GetData() should return pointer to original data (expected: {}, got: "
      "{})",
      static_cast<const void *>(data.data()),
      static_cast<const void *>(buffer.GetData())));

  GOAssert(
    buffer.GetNItems() == nFrames,
    std::format(
      "GetNItems() should return {} (got: {})", nFrames, buffer.GetNItems()));

  GOAssert(
    std::memcmp(buffer.GetData(), data.data(), buffer.GetNBytes()) == 0,
    "Buffer data should match original data");
}

void GOTestSoundBufferMono::TestConvertingConstructor() {
  const unsigned nFrames = 4;

  std::vector<GOSoundBuffer::Item> data(nFrames);

  fillWithSequential(data.data(), nFrames, 5.0f);

  // Converting from a single-channel GOSoundBuffer
  GOSoundBuffer singleChannelBuffer(data.data(), 1, nFrames);
  GOSoundBufferMono fromBuffer(singleChannelBuffer);

  AssertDimensions(
    "ConvertingConstructor from GOSoundBuffer", fromBuffer, 1, nFrames);
  GOAssert(
    fromBuffer.GetData() == data.data(),
    "ConvertingConstructor from GOSoundBuffer should point to same data");

  // Converting from a GOSoundBufferMutableMono, which slices to GOSoundBuffer
  GOSoundBufferMutableMono mutableMono(data.data(), nFrames);
  GOSoundBufferMono fromMutableMono(mutableMono);

  AssertDimensions(
    "ConvertingConstructor from GOSoundBufferMutableMono",
    fromMutableMono,
    1,
    nFrames);
  GOAssert(
    fromMutableMono.GetData() == data.data(),
    "ConvertingConstructor from GOSoundBufferMutableMono should point to "
    "same data");
}

void GOTestSoundBufferMono::TestGetSubBuffer() {
  const unsigned nFrames = 6;

  std::vector<GOSoundBuffer::Item> data(nFrames);

  for (unsigned frameI = 0; frameI < nFrames; ++frameI)
    data[frameI] = static_cast<float>(frameI * 10);

  GOSoundBufferMono buffer(data.data(), nFrames);

  // GetSubBuffer from the beginning
  GOSoundBufferMono sub1 = buffer.GetSubBuffer(0, 2);
  AssertDimensions("GetSubBuffer sub1", sub1, 1, 2);

  GOAssert(
    sub1.GetData() == data.data(),
    std::format(
      "GetSubBuffer at frameIndex 0 should point to same data (expected: {}, "
      "got: {})",
      static_cast<const void *>(data.data()),
      static_cast<const void *>(sub1.GetData())));

  // GetSubBuffer with firstFrameIndex
  const unsigned firstFrameIndex = 2;
  const unsigned subNFrames = 2;
  GOSoundBufferMono sub2 = buffer.GetSubBuffer(firstFrameIndex, subNFrames);

  AssertDimensions("GetSubBuffer", sub2, 1, subNFrames);

  GOAssert(
    sub2.GetData() == data.data() + firstFrameIndex,
    std::format(
      "GetSubBuffer at firstFrameIndex {} should point to correct position",
      firstFrameIndex));

  GOAssert(
    sub2.GetData()[0] == data[firstFrameIndex],
    std::format(
      "First item of GetSubBuffer should be {} (got: {})",
      data[firstFrameIndex],
      sub2.GetData()[0]));
}

void GOTestSoundBufferMono::TestInvalidBuffer() {
  // Buffer with null pointer
  GOSoundBufferMono nullBuffer(nullptr, 4);
  GOAssert(
    !nullBuffer.isValid(), "Buffer with null data pointer should be invalid");

  // Buffer with zero frames
  float dummyData[1];
  GOSoundBufferMono zeroFramesBuffer(dummyData, 0);
  GOAssert(
    !zeroFramesBuffer.isValid(), "Buffer with 0 frames should be invalid");

  // Valid buffer with one frame
  float singleFrame = 1.0f;
  GOSoundBufferMono singleBuffer(&singleFrame, 1);
  GOAssert(singleBuffer.isValid(), "Buffer with 1 frame should be valid");

  GOAssert(
    singleBuffer.GetNItems() == 1,
    std::format(
      "Buffer with 1 frame should have 1 total item (got: {})",
      singleBuffer.GetNItems()));
}

void GOTestSoundBufferMono::TestEdgeCases() {
  const unsigned largeNFrames = 10000;

  std::vector<GOSoundBuffer::Item> largeData(largeNFrames);
  std::fill(largeData.begin(), largeData.end(), 0.5f);

  GOSoundBufferMono largeBuffer(largeData.data(), largeNFrames);

  GOAssert(largeBuffer.isValid(), "Large mono buffer should be valid");

  // GetSubBuffer at the very end
  GOSoundBufferMono lastFrame = largeBuffer.GetSubBuffer(largeNFrames - 1, 1);
  AssertDimensions("EdgeCases lastFrame", lastFrame, 1, 1);

  // GetSubBuffer of entire length
  GOSoundBufferMono fullBuffer = largeBuffer.GetSubBuffer(0, largeNFrames);
  GOAssert(fullBuffer.isValid(), "GetSubBuffer of full length should be valid");

  GOAssert(
    fullBuffer.GetData() == largeData.data(),
    "GetSubBuffer of full length should point to same data");
}

void GOTestSoundBufferMono::TestRoundTripWithPlanarGetChannelBuffer() {
  const unsigned nChannels = 3;
  const unsigned nFrames = 5;
  const unsigned nItems = nChannels * nFrames;

  std::vector<GOSoundBuffer::Item> planarData(nItems);

  fillWithSequential(planarData.data(), nItems, 1.0f);

  GOSoundBufferPlanar planarBuffer(planarData.data(), nChannels, nFrames);

  for (unsigned channelI = 0; channelI < nChannels; ++channelI) {
    GOSoundBufferMono channelBuffer = planarBuffer.GetChannelBuffer(channelI);

    AssertDimensions(
      std::format("RoundTrip channel {}", channelI), channelBuffer, 1, nFrames);

    GOAssert(
      channelBuffer.GetData() == planarData.data() + channelI * nFrames,
      std::format(
        "GetChannelBuffer({}) should point to the start of that channel",
        channelI));

    for (unsigned frameI = 0; frameI < nFrames; ++frameI) {
      const float expectedValue = planarData[channelI * nFrames + frameI];
      const float gotValue = channelBuffer.GetData()[frameI];

      GOAssert(
        gotValue == expectedValue,
        std::format(
          "RoundTrip channel {}, frame {} should be {} (got: {})",
          channelI,
          frameI,
          expectedValue,
          gotValue));
    }
  }
}

void GOTestSoundBufferMono::run() {
  GO_RUN_TEST(TestConstructorAndBasicProperties())
  GO_RUN_TEST(TestConvertingConstructor())
  GO_RUN_TEST(TestGetSubBuffer())
  GO_RUN_TEST(TestInvalidBuffer())
  GO_RUN_TEST(TestEdgeCases())
  GO_RUN_TEST(TestRoundTripWithPlanarGetChannelBuffer())
}
