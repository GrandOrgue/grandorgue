/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2024-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundBuffer.h"

#include <cstring>
#include <format>
#include <numeric>
#include <vector>

#include "sound/buffer/GOSoundBuffer.h"

const std::string GOTestSoundBuffer::TEST_NAME = "GOTestSoundBuffer";

void GOTestSoundBuffer::AssertGetNItems(
  unsigned nChannels, unsigned nFrames, unsigned expectedValue) {
  const unsigned gotValue = GOSoundBuffer::getNItems(nChannels, nFrames);

  GOAssert(
    gotValue == expectedValue,
    std::format(
      "getNItems({}, {}) should return {} (got: {})",
      nChannels,
      nFrames,
      expectedValue,
      gotValue));
}

void GOTestSoundBuffer::TestConstructorAndBasicProperties() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 4;
  const unsigned nItems = nChannels * nFrames;

  std::vector<GOSoundBuffer::Item> data(nItems);
  std::iota(data.begin(), data.end(), 1.0f);

  GOSoundBuffer buffer(data.data(), nChannels, nFrames);

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

  // Check data
  GOAssert(
    std::memcmp(buffer.GetData(), data.data(), buffer.GetNBytes()) == 0,
    "Buffer data should match original data");
}

void GOTestSoundBuffer::TestGetNItems() {
  AssertGetNItems(2, 4, 8);
  AssertGetNItems(1, 10, 10);
  AssertGetNItems(5, 2, 10);
  AssertGetNItems(0, 10, 0);
  AssertGetNItems(2, 0, 0);
}

void GOTestSoundBuffer::AssertGetItemIndex(
  const GOSoundBuffer &buffer,
  unsigned frameIndex,
  unsigned channelIndex,
  unsigned expectedValue) {
  const unsigned gotValue = buffer.GetItemIndex(frameIndex, channelIndex);

  GOAssert(
    gotValue == expectedValue,
    std::format(
      "GetItemIndex({}, {}) should return {} (got: {})",
      frameIndex,
      channelIndex,
      expectedValue,
      gotValue));
}

void GOTestSoundBuffer::TestGetItemIndex() {
  const unsigned nChannels = 3;
  const unsigned nFrames = 4;

  std::vector<GOSoundBuffer::Item> data(nChannels * nFrames);
  std::iota(data.begin(), data.end(), 1.0f);

  GOSoundBuffer buffer(data.data(), nChannels, nFrames);

  AssertGetItemIndex(buffer, 0, 0, 0);
  AssertGetItemIndex(buffer, 0, 1, 1);
  AssertGetItemIndex(buffer, 1, 0, nChannels);
  AssertGetItemIndex(buffer, 1, 1, nChannels + 1);
  AssertGetItemIndex(buffer, 2, 0, nChannels * 2);

  // Test that GetNItems uses GetItemIndex
  AssertGetItemIndex(buffer, nFrames, 0, buffer.GetNItems());
}

void GOTestSoundBuffer::TestGetSubBuffer() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 5;
  const unsigned nItems = nChannels * nFrames;
  std::vector<GOSoundBuffer::Item> data(nItems);

  for (unsigned itemI = 0; itemI < nItems; ++itemI)
    data[itemI] = static_cast<float>(itemI * 10);

  GOSoundBuffer buffer(data.data(), nChannels, nFrames);

  // SubBuffer from the beginning
  GOSoundBuffer sub1 = buffer.GetSubBuffer(0, 2);
  AssertDimensions("GetSubBuffer sub1", sub1, nChannels, 2);

  GOAssert(
    sub1.GetData() == data.data(),
    std::format(
      "SubBuffer at frameIndex 0 should point to same data (expected: {}, got: "
      "{})",
      static_cast<const void *>(data.data()),
      static_cast<const void *>(sub1.GetData())));

  // SubBuffer with firstFrameIndex
  const unsigned firstFrameIndex = 2;
  const unsigned subNFrames = 2;
  GOSoundBuffer sub2 = buffer.GetSubBuffer(firstFrameIndex, subNFrames);

  AssertDimensions("GetSubBuffer", sub2, nChannels, subNFrames);

  const unsigned expectedItemIndex = buffer.GetItemIndex(firstFrameIndex);
  GOAssert(
    sub2.GetData() == data.data() + expectedItemIndex,
    std::format(
      "SubBuffer at firstFrameIndex {} should point to correct position "
      "(expected item index: {} items)",
      firstFrameIndex,
      expectedItemIndex));

  // Check sub-buffer data using GetItemIndex
  GOAssert(
    sub2.GetData()[0] == data[buffer.GetItemIndex(firstFrameIndex, 0)],
    std::format(
      "First item of SubBuffer should be {} (got: {})",
      data[buffer.GetItemIndex(firstFrameIndex, 0)],
      sub2.GetData()[0]));
}

void GOTestSoundBuffer::TestInvalidBuffer() {
  // Buffer with null pointer
  GOSoundBuffer nullBuffer(nullptr, 2, 4);
  GOAssert(
    !nullBuffer.isValid(), "Buffer with null data pointer should be invalid");

  // Buffer with zero channels
  float dummyData[8];
  GOSoundBuffer zeroChannelsBuffer(dummyData, 0, 4);
  GOAssert(
    !zeroChannelsBuffer.isValid(), "Buffer with 0 channels should be invalid");

  // Buffer with zero frames
  GOSoundBuffer zeroFramesBuffer(dummyData, 2, 0);
  GOAssert(
    !zeroFramesBuffer.isValid(), "Buffer with 0 frames should be invalid");

  // Valid buffer with one frame
  float singleFrame[2] = {1.0f, 2.0f};
  GOSoundBuffer singleBuffer(singleFrame, 2, 1);
  GOAssert(singleBuffer.isValid(), "Buffer with 1 frame should be valid");

  GOAssert(
    singleBuffer.GetNItems() == 2,
    std::format(
      "Buffer with 2 channels and 1 frame should have 2 total items (got: {})",
      singleBuffer.GetNItems()));
}

void GOTestSoundBuffer::TestEdgeCases() {
  // Maximum size buffer (within reasonable limits)
  const unsigned largeNChannels = 8;
  const unsigned largeNFrames = 10000;
  const unsigned nItems = largeNChannels * largeNFrames;
  std::vector<GOSoundBuffer::Item> largeData(nItems);

  std::fill(largeData.begin(), largeData.end(), 0.5f);

  GOSoundBuffer largeBuffer(largeData.data(), largeNChannels, largeNFrames);

  GOAssert(largeBuffer.isValid(), "Large buffer should be valid");

  GOAssert(
    largeBuffer.GetNBytes() == nItems * sizeof(GOSoundBuffer::Item),
    std::format(
      "Large buffer size should be {} bytes (got: {})",
      nItems * sizeof(GOSoundBuffer::Item),
      largeBuffer.GetNBytes()));

  // Test GetItemIndex for edge cases
  AssertGetItemIndex(largeBuffer, 0, 0, 0);
  AssertGetItemIndex(
    largeBuffer, largeNFrames - 1, largeNChannels - 1, nItems - 1);

  // SubBuffer at the very end
  GOSoundBuffer lastFrame = largeBuffer.GetSubBuffer(largeNFrames - 1, 1);
  AssertDimensions("EdgeCases lastFrame", lastFrame, largeNChannels, 1);

  // SubBuffer of entire length
  GOSoundBuffer fullBuffer = largeBuffer.GetSubBuffer(0, largeNFrames);
  GOAssert(fullBuffer.isValid(), "SubBuffer of full length should be valid");

  GOAssert(
    fullBuffer.GetData() == largeData.data(),
    "SubBuffer of full length should point to same data");
}

void GOTestSoundBuffer::run() {
  TestConstructorAndBasicProperties();
  TestGetNItems();
  TestGetItemIndex();
  TestGetSubBuffer();
  TestInvalidBuffer();
  TestEdgeCases();
}