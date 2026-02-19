/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2024-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundBufferMutableMono.h"

#include <cstdlib>
#include <cstring>
#include <format>
#include <iostream>
#include <numeric>
#include <vector>

#include "sound/buffer/GOSoundBufferMutableMono.h"

const std::string GOTestSoundBufferMutableMono::TEST_NAME
  = "GOTestSoundBufferMutableMono";

void GOTestSoundBufferMutableMono::TestConstructorAndBasicProperties() {
  const unsigned nFrames = 5;

  std::vector<GOSoundBuffer::Item> data(nFrames);
  std::iota(data.begin(), data.end(), 1.0f);

  GOSoundBufferMutableMono buffer(data.data(), nFrames);

  GOAssert(
    buffer.isValid(),
    "Mono buffer should be valid with non-null data and positive frames");

  GOAssert(
    buffer.GetData() == data.data(),
    std::format(
      "GetData() should return pointer to original data (expected: {}, got: "
      "{})",
      static_cast<const void *>(data.data()),
      static_cast<const void *>(buffer.GetData())));

  GOAssert(
    buffer.GetNChannels() == 1,
    std::format(
      "Mono buffer should have 1 channel (got: {})", buffer.GetNChannels()));

  GOAssert(
    buffer.GetNFrames() == nFrames,
    std::format(
      "GetNFrames() should return {} (got: {})", nFrames, buffer.GetNFrames()));

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
  for (unsigned i = 0; i < nFrames; ++i) {
    data[i] = static_cast<float>(i * 10);
  }

  GOSoundBufferMutableMono buffer(data.data(), nFrames);

  // GetSubBuffer from the beginning
  GOSoundBufferMutableMono sub1 = buffer.GetSubBuffer(0, 2);
  GOAssert(sub1.isValid(), "GetSubBuffer starting at 0 should be valid");

  GOAssert(
    sub1.GetData() == data.data(),
    std::format(
      "GetSubBuffer at frameIndex 0 should point to same data (expected: {}, "
      "got: "
      "{})",
      static_cast<const void *>(data.data()),
      static_cast<const void *>(sub1.GetData())));

  GOAssert(
    sub1.GetNFrames() == 2,
    std::format(
      "GetSubBuffer with 2 frames should have GetNFrames() == 2 (got: {})",
      sub1.GetNFrames()));

  // GetSubBuffer with firstFrameIndex
  const unsigned firstFrameIndex = 2;
  const unsigned subFrames = 2;
  GOSoundBufferMutableMono sub2
    = buffer.GetSubBuffer(firstFrameIndex, subFrames);

  GOAssert(sub2.isValid(), "GetSubBuffer with firstFrameIndex should be valid");

  GOAssert(
    sub2.GetData() == data.data() + firstFrameIndex,
    std::format(
      "GetSubBuffer at firstFrameIndex {} should point to correct position",
      firstFrameIndex));

  GOAssert(
    sub2.GetNFrames() == subFrames,
    std::format(
      "GetSubBuffer should have {} frames (got: {})",
      subFrames,
      sub2.GetNFrames()));

  GOAssert(
    sub2.GetNChannels() == 1,
    std::format(
      "GetSubBuffer should have 1 channel (got: {})", sub2.GetNChannels()));

  // Modify data through sub-buffer
  sub2.GetData()[0] = 99.0f;
  GOAssert(
    data[firstFrameIndex] == 99.0f,
    std::format(
      "Original data should be modified through GetSubBuffer (expected: 99.0, "
      "got: {})",
      data[firstFrameIndex]));
}

void GOTestSoundBufferMutableMono::TestCopyChannelFrom() {
  const unsigned srcChannels = 3;
  const unsigned nSrcFrames = 4;
  const unsigned srcNItems = srcChannels * nSrcFrames;

  // Create multi-channel source buffer
  std::vector<GOSoundBuffer::Item> srcData(srcNItems);
  for (unsigned i = 0; i < srcNItems; ++i) {
    srcData[i] = static_cast<float>(i + 1);
  }

  GOSoundBuffer srcBuffer(srcData.data(), srcChannels, nSrcFrames);

  // Create mono destination buffer
  std::vector<GOSoundBuffer::Item> monoData(nSrcFrames, 0.0f);
  GOSoundBufferMutableMono monoBuffer(monoData.data(), nSrcFrames);

  // Copy from channel 0
  monoBuffer.CopyChannelFrom(srcBuffer, 0);

  // Verify data from channel 0
  for (unsigned i = 0; i < nSrcFrames; ++i) {
    GOAssert(
      monoData[i] == srcData[i * srcChannels],
      std::format(
        "Frame {} should be {} (got: {})",
        i,
        srcData[i * srcChannels],
        monoData[i]));
  }

  // Reset and copy from channel 1
  std::fill(monoData.begin(), monoData.end(), 0.0f);
  monoBuffer.CopyChannelFrom(srcBuffer, 1);

  // Verify data from channel 1
  for (unsigned i = 0; i < nSrcFrames; ++i) {
    GOAssert(
      monoData[i] == srcData[i * srcChannels + 1],
      std::format(
        "Frame {} should be {} (got: {})",
        i,
        srcData[i * srcChannels + 1],
        monoData[i]));
  }

  // Reset and copy from channel 2 (default)
  std::fill(monoData.begin(), monoData.end(), 0.0f);
  monoBuffer.CopyChannelFrom(srcBuffer, 2);

  // Verify data from channel 2
  for (unsigned i = 0; i < nSrcFrames; ++i) {
    GOAssert(
      monoData[i] == srcData[i * srcChannels + 2],
      std::format(
        "Frame {} should be {} (got: {})",
        i,
        srcData[i * srcChannels + 2],
        monoData[i]));
  }
}

void GOTestSoundBufferMutableMono::TestCopyChannelTo() {
  const unsigned dstChannels = 3;
  const unsigned nDstFrames = 4;
  const unsigned dstNItems = dstChannels * nDstFrames;

  // Create mono source buffer
  std::vector<GOSoundBuffer::Item> monoData(nDstFrames);
  for (unsigned i = 0; i < nDstFrames; ++i) {
    monoData[i] = static_cast<float>((i + 1) * 10);
  }

  GOSoundBufferMutableMono monoBuffer(monoData.data(), nDstFrames);

  // Create multi-channel destination buffer
  std::vector<GOSoundBuffer::Item> dstData(dstNItems, 0.0f);
  GOSoundBufferMutable dstBuffer(dstData.data(), dstChannels, nDstFrames);

  // Copy to channel 0
  monoBuffer.CopyChannelTo(dstBuffer, 0);

  // Verify data in channel 0
  for (unsigned i = 0; i < nDstFrames; ++i) {
    GOAssert(
      dstData[i * dstChannels] == monoData[i],
      std::format(
        "Channel 0, frame {} should be {} (got: {})",
        i,
        monoData[i],
        dstData[i * dstChannels]));
  }

  // Copy to channel 1
  monoBuffer.CopyChannelTo(dstBuffer, 1);

  // Verify data in channel 1
  for (unsigned i = 0; i < nDstFrames; ++i) {
    GOAssert(
      dstData[i * dstChannels + 1] == monoData[i],
      std::format(
        "Channel 1, frame {} should be {} (got: {})",
        i,
        monoData[i],
        dstData[i * dstChannels + 1]));
  }

  // Copy to channel 2 (default)
  monoBuffer.CopyChannelTo(dstBuffer, 2);

  // Verify data in channel 2
  for (unsigned i = 0; i < nDstFrames; ++i) {
    GOAssert(
      dstData[i * dstChannels + 2] == monoData[i],
      std::format(
        "Channel 2, frame {} should be {} (got: {})",
        i,
        monoData[i],
        dstData[i * dstChannels + 2]));
  }
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
      "Buffer with 1 frame should have 1 total unit (got: {})",
      singleBuffer.GetNItems()));
}

void GOTestSoundBufferMutableMono::TestEdgeCases() {
  // Large mono buffer
  const unsigned largeFrames = 10000;

  std::vector<GOSoundBuffer::Item> largeData(largeFrames);
  std::fill(largeData.begin(), largeData.end(), 0.5f);

  GOSoundBufferMutableMono largeBuffer(largeData.data(), largeFrames);

  GOAssert(largeBuffer.isValid(), "Large mono buffer should be valid");

  // GetSubBuffer at the very end
  GOSoundBufferMutableMono lastFrame
    = largeBuffer.GetSubBuffer(largeFrames - 1, 1);
  GOAssert(lastFrame.isValid(), "GetSubBuffer at last frame should be valid");

  GOAssert(
    lastFrame.GetNFrames() == 1,
    std::format(
      "Last frame GetSubBuffer should have 1 frame (got: {})",
      lastFrame.GetNFrames()));

  // GetSubBuffer of entire length
  GOSoundBufferMutableMono fullBuffer
    = largeBuffer.GetSubBuffer(0, largeFrames);
  GOAssert(fullBuffer.isValid(), "GetSubBuffer of full length should be valid");

  GOAssert(
    fullBuffer.GetData() == largeData.data(),
    "GetSubBuffer of full length should point to same data");

  // Test CopyChannelFrom with stereo buffer
  const unsigned stereoChannels = 2;
  const unsigned stereoDataSize = stereoChannels * largeFrames;
  std::vector<GOSoundBuffer::Item> stereoData(stereoDataSize);
  for (unsigned i = 0; i < stereoDataSize; ++i) {
    stereoData[i] = static_cast<float>(i % 100);
  }

  GOSoundBuffer stereoBuffer(stereoData.data(), stereoChannels, largeFrames);

  // Copy from left channel
  std::vector<GOSoundBuffer::Item> monoResult(largeFrames, 0.0f);
  GOSoundBufferMutableMono monoResultBuffer(monoResult.data(), largeFrames);
  monoResultBuffer.CopyChannelFrom(stereoBuffer, 0);

  // Verify left channel data
  for (unsigned i = 0; i < largeFrames; ++i) {
    GOAssert(
      monoResult[i] == stereoData[i * stereoChannels],
      std::format("Left channel frame {} mismatch", i));
  }
}

void GOTestSoundBufferMutableMono::run() {
  TestConstructorAndBasicProperties();
  TestGetSubBuffer();
  TestCopyChannelFrom();
  TestCopyChannelTo();
  TestInvalidBuffer();
  TestEdgeCases();
}