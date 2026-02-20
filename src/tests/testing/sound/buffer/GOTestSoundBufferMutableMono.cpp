/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2024-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundBufferMutableMono.h"

#include <cstring>
#include <format>
#include <numeric>

#include "sound/buffer/GOSoundBufferMutableMono.h"

const std::string GOTestSoundBufferMutableMono::TEST_NAME
  = "GOTestSoundBufferMutableMono";

void GOTestSoundBufferMutableMono::TestConstructorAndBasicProperties() {
  const unsigned nFrames = 5;

  std::vector<GOSoundBuffer::Item> data(nFrames);
  std::iota(data.begin(), data.end(), 1.0f);

  GOSoundBufferMutableMono buffer(data.data(), nFrames);

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

  // Check data
  GOAssert(
    std::memcmp(buffer.GetData(), data.data(), buffer.GetNBytes()) == 0,
    "Buffer data should match original data");
}

void GOTestSoundBufferMutableMono::TestGetSubBuffer() {
  const unsigned nFrames = 6;

  std::vector<GOSoundBuffer::Item> data(nFrames);

  for (unsigned frameI = 0; frameI < nFrames; ++frameI)
    data[frameI] = static_cast<float>(frameI * 10);

  GOSoundBufferMutableMono buffer(data.data(), nFrames);

  // GetSubBuffer from the beginning
  GOSoundBufferMutableMono sub1 = buffer.GetSubBuffer(0, 2);
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
  GOSoundBufferMutableMono sub2
    = buffer.GetSubBuffer(firstFrameIndex, subNFrames);

  AssertDimensions("GetSubBuffer", sub2, 1, subNFrames);

  GOAssert(
    sub2.GetData() == data.data() + firstFrameIndex,
    std::format(
      "GetSubBuffer at firstFrameIndex {} should point to correct position",
      firstFrameIndex));

  // Modify data through sub-buffer
  sub2.GetData()[0] = 99.0f;
  GOAssert(
    data[firstFrameIndex] == 99.0f,
    std::format(
      "Original data should be modified through GetSubBuffer (expected: 99.0, "
      "got: {})",
      data[firstFrameIndex]));
}

void GOTestSoundBufferMutableMono::TestCopyMonoFromChannel(
  GOSoundBufferMutableMono &monoBuffer,
  std::vector<GOSoundBuffer::Item> &monoData,
  const GOSoundBuffer &srcBuffer,
  unsigned srcNChannels,
  unsigned channelI,
  unsigned nFrames) {
  std::fill(monoData.begin(), monoData.end(), 0.0f);
  monoBuffer.CopyChannelFrom(srcBuffer, channelI);

  for (unsigned frameI = 0; frameI < nFrames; ++frameI)
    GOAssert(
      monoData[frameI] == srcBuffer.GetData()[frameI * srcNChannels + channelI],
      std::format(
        "Channel {}, frame {} should be {} (got: {})",
        channelI,
        frameI,
        srcBuffer.GetData()[frameI * srcNChannels + channelI],
        monoData[frameI]));
}

void GOTestSoundBufferMutableMono::TestCopyMonoToChannel(
  GOSoundBufferMutableMono &monoBuffer,
  const std::vector<GOSoundBuffer::Item> &monoData,
  GOSoundBufferMutable &dstBuffer,
  const std::vector<GOSoundBuffer::Item> &dstData,
  unsigned dstNChannels,
  unsigned channelI,
  unsigned nFrames) {
  monoBuffer.CopyChannelTo(dstBuffer, channelI);

  for (unsigned frameI = 0; frameI < nFrames; ++frameI)
    GOAssert(
      dstData[frameI * dstNChannels + channelI] == monoData[frameI],
      std::format(
        "Channel {}, frame {} should be {} (got: {})",
        channelI,
        frameI,
        monoData[frameI],
        dstData[frameI * dstNChannels + channelI]));
}

void GOTestSoundBufferMutableMono::TestCopyChannelFrom() {
  const unsigned srcNChannels = 3;
  const unsigned nSrcFrames = 4;
  const unsigned srcNItems = srcNChannels * nSrcFrames;

  // Create multi-channel source buffer
  std::vector<GOSoundBuffer::Item> srcData(srcNItems);

  fillWithSequential(srcData.data(), srcNItems, 1.0f);

  GOSoundBuffer srcBuffer(srcData.data(), srcNChannels, nSrcFrames);

  // Create mono destination buffer
  std::vector<GOSoundBuffer::Item> monoData(nSrcFrames, 0.0f);
  GOSoundBufferMutableMono monoBuffer(monoData.data(), nSrcFrames);

  TestCopyMonoFromChannel(
    monoBuffer, monoData, srcBuffer, srcNChannels, 0, nSrcFrames);
  TestCopyMonoFromChannel(
    monoBuffer, monoData, srcBuffer, srcNChannels, 1, nSrcFrames);
  TestCopyMonoFromChannel(
    monoBuffer, monoData, srcBuffer, srcNChannels, 2, nSrcFrames);
}

void GOTestSoundBufferMutableMono::TestCopyChannelTo() {
  const unsigned dstNChannels = 3;
  const unsigned nDstFrames = 4;
  const unsigned dstNItems = dstNChannels * nDstFrames;

  // Create mono source buffer
  std::vector<GOSoundBuffer::Item> monoData(nDstFrames);

  for (unsigned frameI = 0; frameI < nDstFrames; ++frameI)
    monoData[frameI] = static_cast<float>((frameI + 1) * 10);

  GOSoundBufferMutableMono monoBuffer(monoData.data(), nDstFrames);

  // Create multi-channel destination buffer
  std::vector<GOSoundBuffer::Item> dstData(dstNItems, 0.0f);
  GOSoundBufferMutable dstBuffer(dstData.data(), dstNChannels, nDstFrames);

  TestCopyMonoToChannel(
    monoBuffer, monoData, dstBuffer, dstData, dstNChannels, 0, nDstFrames);
  TestCopyMonoToChannel(
    monoBuffer, monoData, dstBuffer, dstData, dstNChannels, 1, nDstFrames);
  TestCopyMonoToChannel(
    monoBuffer, monoData, dstBuffer, dstData, dstNChannels, 2, nDstFrames);
}

void GOTestSoundBufferMutableMono::TestInvalidBuffer() {
  // Buffer with null pointer
  GOSoundBufferMutableMono nullBuffer(nullptr, 4);
  GOAssert(
    !nullBuffer.isValid(), "Buffer with null data pointer should be invalid");

  // Buffer with zero frames
  float dummyData[1];
  GOSoundBufferMutableMono zeroFramesBuffer(dummyData, 0);
  GOAssert(
    !zeroFramesBuffer.isValid(), "Buffer with 0 frames should be invalid");

  // Valid buffer with one frame
  float singleFrame = 1.0f;
  GOSoundBufferMutableMono singleBuffer(&singleFrame, 1);
  GOAssert(singleBuffer.isValid(), "Buffer with 1 frame should be valid");

  GOAssert(
    singleBuffer.GetNItems() == 1,
    std::format(
      "Buffer with 1 frame should have 1 total item (got: {})",
      singleBuffer.GetNItems()));
}

void GOTestSoundBufferMutableMono::TestEdgeCases() {
  // Large mono buffer
  const unsigned largeNFrames = 10000;

  std::vector<GOSoundBuffer::Item> largeData(largeNFrames);
  std::fill(largeData.begin(), largeData.end(), 0.5f);

  GOSoundBufferMutableMono largeBuffer(largeData.data(), largeNFrames);

  GOAssert(largeBuffer.isValid(), "Large mono buffer should be valid");

  // GetSubBuffer at the very end
  GOSoundBufferMutableMono lastFrame
    = largeBuffer.GetSubBuffer(largeNFrames - 1, 1);
  AssertDimensions("EdgeCases lastFrame", lastFrame, 1, 1);

  // GetSubBuffer of entire length
  GOSoundBufferMutableMono fullBuffer
    = largeBuffer.GetSubBuffer(0, largeNFrames);
  GOAssert(fullBuffer.isValid(), "GetSubBuffer of full length should be valid");

  GOAssert(
    fullBuffer.GetData() == largeData.data(),
    "GetSubBuffer of full length should point to same data");

  // Test CopyChannelFrom with stereo buffer
  const unsigned stereoNChannels = 2;
  const unsigned stereoNItems = stereoNChannels * largeNFrames;
  std::vector<GOSoundBuffer::Item> stereoData(stereoNItems);

  for (unsigned itemI = 0; itemI < stereoNItems; ++itemI)
    stereoData[itemI] = static_cast<float>(itemI % 100);

  GOSoundBuffer stereoBuffer(stereoData.data(), stereoNChannels, largeNFrames);

  // Copy from left channel
  std::vector<GOSoundBuffer::Item> monoResult(largeNFrames, 0.0f);
  GOSoundBufferMutableMono monoResultBuffer(monoResult.data(), largeNFrames);

  monoResultBuffer.CopyChannelFrom(stereoBuffer, 0);

  // Verify left channel data
  for (unsigned frameI = 0; frameI < largeNFrames; ++frameI)
    GOAssert(
      monoResult[frameI] == stereoData[frameI * stereoNChannels],
      std::format("Left channel frame {} mismatch", frameI));
}

void GOTestSoundBufferMutableMono::run() {
  TestConstructorAndBasicProperties();
  TestGetSubBuffer();
  TestCopyChannelFrom();
  TestCopyChannelTo();
  TestInvalidBuffer();
  TestEdgeCases();
}