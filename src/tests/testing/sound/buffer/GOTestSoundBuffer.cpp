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
  const unsigned nSamples = 4;
  const unsigned totalUnits = nChannels * nSamples;

  std::vector<GOSoundBuffer::SoundUnit> data(totalUnits);
  std::iota(data.begin(), data.end(), 1.0f);

  GOSoundBuffer buffer(data.data(), nChannels, nSamples);

  GOAssert(
    buffer.isValid(),
    "Buffer should be valid with non-null data, positive channels and samples");

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
    buffer.GetNSamples() == nSamples,
    std::format(
      "GetNSamples() should return {} (got: {})",
      nSamples,
      buffer.GetNSamples()));

  GOAssert(
    buffer.GetNUnits() == totalUnits,
    std::format(
      "GetNUnits() should return {} (got: {})",
      totalUnits,
      buffer.GetNUnits()));

  GOAssert(
    buffer.GetNBytes() == totalUnits * sizeof(GOSoundBuffer::SoundUnit),
    std::format(
      "GetNBytes() should return {} (got: {})",
      totalUnits * sizeof(GOSoundBuffer::SoundUnit),
      buffer.GetNBytes()));

  // Check data
  GOAssert(
    std::memcmp(buffer.GetData(), data.data(), buffer.GetNBytes()) == 0,
    "Buffer data should match original data");
}

void GOTestSoundBuffer::TestGetNUnits() {
  GOAssert(
    GOSoundBuffer::getNUnits(2, 4) == 8,
    std::format(
      "getNUnits(2, 4) should return 8 (got: {})",
      GOSoundBuffer::getNUnits(2, 4)));

  GOAssert(
    GOSoundBuffer::getNUnits(1, 10) == 10,
    std::format(
      "getNUnits(1, 10) should return 10 (got: {})",
      GOSoundBuffer::getNUnits(1, 10)));

  GOAssert(
    GOSoundBuffer::getNUnits(5, 2) == 10,
    std::format(
      "getNUnits(5, 2) should return 10 (got: {})",
      GOSoundBuffer::getNUnits(5, 2)));

  GOAssert(
    GOSoundBuffer::getNUnits(0, 10) == 0,
    std::format(
      "getNUnits(0, 10) should return 0 (got: {})",
      GOSoundBuffer::getNUnits(0, 10)));

  GOAssert(
    GOSoundBuffer::getNUnits(2, 0) == 0,
    std::format(
      "getNUnits(2, 0) should return 0 (got: {})",
      GOSoundBuffer::getNUnits(2, 0)));
}

void GOTestSoundBuffer::TestGetUnitOffset() {
  const unsigned nChannels = 3;
  const unsigned nSamples = 4;

  std::vector<GOSoundBuffer::SoundUnit> data(nChannels * nSamples);
  std::iota(data.begin(), data.end(), 1.0f);

  GOSoundBuffer buffer(data.data(), nChannels, nSamples);

  // Test basic offsets
  GOAssert(
    buffer.GetUnitOffset(0, 0) == 0,
    std::format(
      "GetUnitOffset(0, 0) should return 0 (got: {})",
      buffer.GetUnitOffset(0, 0)));

  GOAssert(
    buffer.GetUnitOffset(0, 1) == 1,
    std::format(
      "GetUnitOffset(0, 1) should return 1 (got: {})",
      buffer.GetUnitOffset(0, 1)));

  GOAssert(
    buffer.GetUnitOffset(1, 0) == nChannels,
    std::format(
      "GetUnitOffset(1, 0) should return {} (got: {})",
      nChannels,
      buffer.GetUnitOffset(1, 0)));

  GOAssert(
    buffer.GetUnitOffset(1, 1) == nChannels + 1,
    std::format(
      "GetUnitOffset(1, 1) should return {} (got: {})",
      nChannels + 1,
      buffer.GetUnitOffset(1, 1)));

  GOAssert(
    buffer.GetUnitOffset(2) == nChannels * 2,
    std::format(
      "GetUnitOffset(2) should return {} (got: {})",
      nChannels * 2,
      buffer.GetUnitOffset(2)));

  // Test that GetNUnits uses GetUnitOffset
  GOAssert(
    buffer.GetNUnits() == buffer.GetUnitOffset(nSamples, 0),
    std::format("GetNUnits() should equal GetUnitOffset({}, 0)", nSamples));
}

void GOTestSoundBuffer::TestGetSubBuffer() {
  const unsigned nChannels = 2;
  const unsigned nSamples = 5;
  const unsigned totalUnits = nChannels * nSamples;

  std::vector<GOSoundBuffer::SoundUnit> data(totalUnits);
  for (unsigned i = 0; i < totalUnits; ++i) {
    data[i] = static_cast<float>(i * 10);
  }

  GOSoundBuffer buffer(data.data(), nChannels, nSamples);

  // SubBuffer from the beginning
  GOSoundBuffer sub1 = buffer.GetSubBuffer(0, 2);
  GOAssert(sub1.isValid(), "SubBuffer starting at 0 should be valid");

  GOAssert(
    sub1.GetData() == data.data(),
    std::format(
      "SubBuffer at offset 0 should point to same data (expected: {}, got: {})",
      static_cast<const void *>(data.data()),
      static_cast<const void *>(sub1.GetData())));

  GOAssert(
    sub1.GetNSamples() == 2,
    std::format(
      "SubBuffer with 2 samples should have GetNSamples() == 2 (got: {})",
      sub1.GetNSamples()));

  // SubBuffer with offset
  const unsigned offset = 2;
  const unsigned subSamples = 2;
  GOSoundBuffer sub2 = buffer.GetSubBuffer(offset, subSamples);

  GOAssert(sub2.isValid(), "SubBuffer with offset should be valid");

  const unsigned expectedOffsetUnits = buffer.GetUnitOffset(offset);
  GOAssert(
    sub2.GetData() == data.data() + expectedOffsetUnits,
    std::format(
      "SubBuffer at offset {} should point to correct position (expected "
      "offset: {} units)",
      offset,
      expectedOffsetUnits));

  GOAssert(
    sub2.GetNSamples() == subSamples,
    std::format(
      "SubBuffer should have {} samples (got: {})",
      subSamples,
      sub2.GetNSamples()));

  GOAssert(
    sub2.GetNChannels() == nChannels,
    std::format(
      "SubBuffer should have {} channels (got: {})",
      nChannels,
      sub2.GetNChannels()));

  // Check sub-buffer data using GetUnitOffset
  GOAssert(
    sub2.GetData()[0] == data[buffer.GetUnitOffset(offset, 0)],
    std::format(
      "First unit of SubBuffer should be {} (got: {})",
      data[buffer.GetUnitOffset(offset, 0)],
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

  // Buffer with zero samples
  GOSoundBuffer zeroSamplesBuffer(dummyData, 2, 0);
  GOAssert(
    !zeroSamplesBuffer.isValid(), "Buffer with 0 samples should be invalid");

  // Valid buffer with one sample
  float singleSample[2] = {1.0f, 2.0f};
  GOSoundBuffer singleBuffer(singleSample, 2, 1);
  GOAssert(singleBuffer.isValid(), "Buffer with 1 sample should be valid");

  GOAssert(
    singleBuffer.GetNUnits() == 2,
    std::format(
      "Buffer with 2 channels and 1 sample should have 2 total units (got: {})",
      singleBuffer.GetNUnits()));
}

void GOTestSoundBuffer::TestEdgeCases() {
  // Maximum size buffer (within reasonable limits)
  const unsigned largeChannels = 8;
  const unsigned largeSamples = 10000;
  const unsigned largeTotalUnits = largeChannels * largeSamples;

  std::vector<GOSoundBuffer::SoundUnit> largeData(largeTotalUnits);
  std::fill(largeData.begin(), largeData.end(), 0.5f);

  GOSoundBuffer largeBuffer(largeData.data(), largeChannels, largeSamples);

  GOAssert(largeBuffer.isValid(), "Large buffer should be valid");

  GOAssert(
    largeBuffer.GetNBytes()
      == largeTotalUnits * sizeof(GOSoundBuffer::SoundUnit),
    std::format(
      "Large buffer size should be {} bytes (got: {})",
      largeTotalUnits * sizeof(GOSoundBuffer::SoundUnit),
      largeBuffer.GetNBytes()));

  // Test GetUnitOffset for edge cases
  GOAssert(
    largeBuffer.GetUnitOffset(0, 0) == 0,
    "GetUnitOffset(0, 0) should return 0 for large buffer");

  GOAssert(
    largeBuffer.GetUnitOffset(largeSamples - 1, largeChannels - 1)
      == largeTotalUnits - 1,
    std::format(
      "GetUnitOffset({}, {}) should return last unit index {}",
      largeSamples - 1,
      largeChannels - 1,
      largeTotalUnits - 1));

  // SubBuffer at the very end
  GOSoundBuffer lastSample = largeBuffer.GetSubBuffer(largeSamples - 1, 1);
  GOAssert(lastSample.isValid(), "SubBuffer at last sample should be valid");

  GOAssert(
    lastSample.GetNSamples() == 1,
    std::format(
      "Last sample SubBuffer should have 1 sample (got: {})",
      lastSample.GetNSamples()));

  // SubBuffer of entire length
  GOSoundBuffer fullBuffer = largeBuffer.GetSubBuffer(0, largeSamples);
  GOAssert(fullBuffer.isValid(), "SubBuffer of full length should be valid");

  GOAssert(
    fullBuffer.GetData() == largeData.data(),
    "SubBuffer of full length should point to same data");
}

void GOTestSoundBuffer::run() {
  TestConstructorAndBasicProperties();
  TestGetNUnits();
  TestGetUnitOffset();
  TestGetSubBuffer();
  TestInvalidBuffer();
  TestEdgeCases();
}