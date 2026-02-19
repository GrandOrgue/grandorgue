/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2024-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundBuffer.h"

#include <cstdlib>
#include <cstring>
#include <format>
#include <iostream>
#include <numeric>
#include <vector>

#include "sound/buffer/GOSoundBuffer.h"

const std::string GOTestSoundBuffer::TEST_NAME = "GOTestSoundBuffer";

void GOTestSoundBuffer::TestConstructorAndBasicProperties() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 4;
  const unsigned totalItems = nChannels * nFrames;

  std::vector<GOSoundBuffer::Item> data(totalItems);
  std::iota(data.begin(), data.end(), 1.0f);

  GOSoundBuffer buffer(data.data(), nChannels, nFrames);

  GOAssert(
    buffer.isValid(),
    "Buffer should be valid with non-null data, positive channels and frames");

  GOAssert(
    buffer.GetData() == data.data(),
    std::format(
      "GetData() should return pointer to original data (expected: {}, got: "
      "{})",
      static_cast<const void *>(data.data()),
      static_cast<const void *>(buffer.GetData())));

  GOAssert(
    buffer.GetNChannels() == nChannels,
    std::format(
      "GetNChannels() should return {} (got: {})",
      nChannels,
      buffer.GetNChannels()));

  GOAssert(
    buffer.GetNFrames() == nFrames,
    std::format(
      "GetNFrames() should return {} (got: {})", nFrames, buffer.GetNFrames()));

  GOAssert(
    buffer.GetNItems() == totalItems,
    std::format(
      "GetNItems() should return {} (got: {})",
      totalItems,
      buffer.GetNItems()));

  GOAssert(
    buffer.GetNBytes() == totalItems * sizeof(GOSoundBuffer::Item),
    std::format(
      "GetNBytes() should return {} (got: {})",
      totalItems * sizeof(GOSoundBuffer::Item),
      buffer.GetNBytes()));

  // Check data
  GOAssert(
    std::memcmp(buffer.GetData(), data.data(), buffer.GetNBytes()) == 0,
    "Buffer data should match original data");
}

void GOTestSoundBuffer::TestGetNItems() {
  GOAssert(
    GOSoundBuffer::getNItems(2, 4) == 8,
    std::format(
      "getNItems(2, 4) should return 8 (got: {})",
      GOSoundBuffer::getNItems(2, 4)));

  GOAssert(
    GOSoundBuffer::getNItems(1, 10) == 10,
    std::format(
      "getNItems(1, 10) should return 10 (got: {})",
      GOSoundBuffer::getNItems(1, 10)));

  GOAssert(
    GOSoundBuffer::getNItems(5, 2) == 10,
    std::format(
      "getNItems(5, 2) should return 10 (got: {})",
      GOSoundBuffer::getNItems(5, 2)));

  GOAssert(
    GOSoundBuffer::getNItems(0, 10) == 0,
    std::format(
      "getNItems(0, 10) should return 0 (got: {})",
      GOSoundBuffer::getNItems(0, 10)));

  GOAssert(
    GOSoundBuffer::getNItems(2, 0) == 0,
    std::format(
      "getNItems(2, 0) should return 0 (got: {})",
      GOSoundBuffer::getNItems(2, 0)));
}

void GOTestSoundBuffer::TestGetItemIndex() {
  const unsigned nChannels = 3;
  const unsigned nFrames = 4;

  std::vector<GOSoundBuffer::Item> data(nChannels * nFrames);
  std::iota(data.begin(), data.end(), 1.0f);

  GOSoundBuffer buffer(data.data(), nChannels, nFrames);

  // Test basic indices
  GOAssert(
    buffer.GetItemIndex(0, 0) == 0,
    std::format(
      "GetItemIndex(0, 0) should return 0 (got: {})",
      buffer.GetItemIndex(0, 0)));

  GOAssert(
    buffer.GetItemIndex(0, 1) == 1,
    std::format(
      "GetItemIndex(0, 1) should return 1 (got: {})",
      buffer.GetItemIndex(0, 1)));

  GOAssert(
    buffer.GetItemIndex(1, 0) == nChannels,
    std::format(
      "GetItemIndex(1, 0) should return {} (got: {})",
      nChannels,
      buffer.GetItemIndex(1, 0)));

  GOAssert(
    buffer.GetItemIndex(1, 1) == nChannels + 1,
    std::format(
      "GetItemIndex(1, 1) should return {} (got: {})",
      nChannels + 1,
      buffer.GetItemIndex(1, 1)));

  GOAssert(
    buffer.GetItemIndex(2) == nChannels * 2,
    std::format(
      "GetItemIndex(2) should return {} (got: {})",
      nChannels * 2,
      buffer.GetItemIndex(2)));

  // Test that GetNItems uses GetItemIndex
  GOAssert(
    buffer.GetNItems() == buffer.GetItemIndex(nFrames, 0),
    std::format("GetNItems() should equal GetItemIndex({}, 0)", nFrames));
}

void GOTestSoundBuffer::TestGetSubBuffer() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 5;
  const unsigned totalItems = nChannels * nFrames;

  std::vector<GOSoundBuffer::Item> data(totalItems);
  for (unsigned i = 0; i < totalItems; ++i) {
    data[i] = static_cast<float>(i * 10);
  }

  GOSoundBuffer buffer(data.data(), nChannels, nFrames);

  // SubBuffer from the beginning
  GOSoundBuffer sub1 = buffer.GetSubBuffer(0, 2);
  GOAssert(sub1.isValid(), "SubBuffer starting at 0 should be valid");

  GOAssert(
    sub1.GetData() == data.data(),
    std::format(
      "SubBuffer at frameIndex 0 should point to same data (expected: {}, got: "
      "{})",
      static_cast<const void *>(data.data()),
      static_cast<const void *>(sub1.GetData())));

  GOAssert(
    sub1.GetNFrames() == 2,
    std::format(
      "SubBuffer with 2 frames should have GetNFrames() == 2 (got: {})",
      sub1.GetNFrames()));

  // SubBuffer with firstFrameIndex
  const unsigned firstFrameIndex = 2;
  const unsigned subFrames = 2;
  GOSoundBuffer sub2 = buffer.GetSubBuffer(firstFrameIndex, subFrames);

  GOAssert(sub2.isValid(), "SubBuffer with firstFrameIndex should be valid");

  const unsigned expectedItemIndex = buffer.GetItemIndex(firstFrameIndex);
  GOAssert(
    sub2.GetData() == data.data() + expectedItemIndex,
    std::format(
      "SubBuffer at firstFrameIndex {} should point to correct position "
      "(expected item index: {} items)",
      firstFrameIndex,
      expectedItemIndex));

  GOAssert(
    sub2.GetNFrames() == subFrames,
    std::format(
      "SubBuffer should have {} frames (got: {})",
      subFrames,
      sub2.GetNFrames()));

  GOAssert(
    sub2.GetNChannels() == nChannels,
    std::format(
      "SubBuffer should have {} channels (got: {})",
      nChannels,
      sub2.GetNChannels()));

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
  const unsigned largeChannels = 8;
  const unsigned largeFrames = 10000;
  const unsigned largeTotalItems = largeChannels * largeFrames;

  std::vector<GOSoundBuffer::Item> largeData(largeTotalItems);
  std::fill(largeData.begin(), largeData.end(), 0.5f);

  GOSoundBuffer largeBuffer(largeData.data(), largeChannels, largeFrames);

  GOAssert(largeBuffer.isValid(), "Large buffer should be valid");

  GOAssert(
    largeBuffer.GetNBytes() == largeTotalItems * sizeof(GOSoundBuffer::Item),
    std::format(
      "Large buffer size should be {} bytes (got: {})",
      largeTotalItems * sizeof(GOSoundBuffer::Item),
      largeBuffer.GetNBytes()));

  // Test GetItemIndex for edge cases
  GOAssert(
    largeBuffer.GetItemIndex(0, 0) == 0,
    "GetItemIndex(0, 0) should return 0 for large buffer");

  GOAssert(
    largeBuffer.GetItemIndex(largeFrames - 1, largeChannels - 1)
      == largeTotalItems - 1,
    std::format(
      "GetItemIndex({}, {}) should return last item index {}",
      largeFrames - 1,
      largeChannels - 1,
      largeTotalItems - 1));

  // SubBuffer at the very end
  GOSoundBuffer lastFrame = largeBuffer.GetSubBuffer(largeFrames - 1, 1);
  GOAssert(lastFrame.isValid(), "SubBuffer at last frame should be valid");

  GOAssert(
    lastFrame.GetNFrames() == 1,
    std::format(
      "Last frame SubBuffer should have 1 frame (got: {})",
      lastFrame.GetNFrames()));

  // SubBuffer of entire length
  GOSoundBuffer fullBuffer = largeBuffer.GetSubBuffer(0, largeFrames);
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