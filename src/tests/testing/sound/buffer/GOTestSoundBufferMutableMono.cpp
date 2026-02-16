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
  const unsigned nSamples = 5;

  std::vector<GOSoundBuffer::SoundUnit> data(nSamples);
  std::iota(data.begin(), data.end(), 1.0f);

  GOSoundBufferMutableMono buffer(data.data(), nSamples);

  GOAssert(
    buffer.isValid(),
    "Mono buffer should be valid with non-null data and positive samples");

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
    buffer.GetNSamples() == nSamples,
    std::format(
      "GetNSamples() should return {} (got: {})",
      nSamples,
      buffer.GetNSamples()));

  GOAssert(
    buffer.GetNUnits() == nSamples,
    std::format(
      "GetNUnits() should return {} (got: {})", nSamples, buffer.GetNUnits()));

  // Check data
  GOAssert(
    std::memcmp(buffer.GetData(), data.data(), buffer.GetNBytes()) == 0,
    "Buffer data should match original data");
}

void GOTestSoundBufferMutableMono::TestGetSubBuffer() {
  const unsigned nSamples = 6;

  std::vector<GOSoundBuffer::SoundUnit> data(nSamples);
  for (unsigned i = 0; i < nSamples; ++i) {
    data[i] = static_cast<float>(i * 10);
  }

  GOSoundBufferMutableMono buffer(data.data(), nSamples);

  // GetSubBuffer from the beginning
  GOSoundBufferMutableMono sub1 = buffer.GetSubBuffer(0, 2);
  GOAssert(sub1.isValid(), "GetSubBuffer starting at 0 should be valid");

  GOAssert(
    sub1.GetData() == data.data(),
    std::format(
      "GetSubBuffer at offset 0 should point to same data (expected: {}, got: "
      "{})",
      static_cast<const void *>(data.data()),
      static_cast<const void *>(sub1.GetData())));

  GOAssert(
    sub1.GetNSamples() == 2,
    std::format(
      "GetSubBuffer with 2 samples should have GetNSamples() == 2 (got: {})",
      sub1.GetNSamples()));

  // GetSubBuffer with offset
  const unsigned offset = 2;
  const unsigned subSamples = 2;
  GOSoundBufferMutableMono sub2 = buffer.GetSubBuffer(offset, subSamples);

  GOAssert(sub2.isValid(), "GetSubBuffer with offset should be valid");

  GOAssert(
    sub2.GetData() == data.data() + offset,
    std::format(
      "GetSubBuffer at offset {} should point to correct position", offset));

  GOAssert(
    sub2.GetNSamples() == subSamples,
    std::format(
      "GetSubBuffer should have {} samples (got: {})",
      subSamples,
      sub2.GetNSamples()));

  GOAssert(
    sub2.GetNChannels() == 1,
    std::format(
      "GetSubBuffer should have 1 channel (got: {})", sub2.GetNChannels()));

  // Modify data through sub-buffer
  sub2.GetData()[0] = 99.0f;
  GOAssert(
    data[offset] == 99.0f,
    std::format(
      "Original data should be modified through GetSubBuffer (expected: 99.0, "
      "got: {})",
      data[offset]));
}

void GOTestSoundBufferMutableMono::TestCopyChannelFrom() {
  const unsigned srcChannels = 3;
  const unsigned srcSamples = 4;
  const unsigned srcTotalUnits = srcChannels * srcSamples;

  // Create multi-channel source buffer
  std::vector<GOSoundBuffer::SoundUnit> srcData(srcTotalUnits);
  for (unsigned i = 0; i < srcTotalUnits; ++i) {
    srcData[i] = static_cast<float>(i + 1);
  }

  GOSoundBuffer srcBuffer(srcData.data(), srcChannels, srcSamples);

  // Create mono destination buffer
  std::vector<GOSoundBuffer::SoundUnit> monoData(srcSamples, 0.0f);
  GOSoundBufferMutableMono monoBuffer(monoData.data(), srcSamples);

  // Copy from channel 0
  monoBuffer.CopyChannelFrom(srcBuffer, 0);

  // Verify data from channel 0
  for (unsigned i = 0; i < srcSamples; ++i) {
    GOAssert(
      monoData[i] == srcData[i * srcChannels],
      std::format(
        "Sample {} should be {} (got: {})",
        i,
        srcData[i * srcChannels],
        monoData[i]));
  }

  // Reset and copy from channel 1
  std::fill(monoData.begin(), monoData.end(), 0.0f);
  monoBuffer.CopyChannelFrom(srcBuffer, 1);

  // Verify data from channel 1
  for (unsigned i = 0; i < srcSamples; ++i) {
    GOAssert(
      monoData[i] == srcData[i * srcChannels + 1],
      std::format(
        "Sample {} should be {} (got: {})",
        i,
        srcData[i * srcChannels + 1],
        monoData[i]));
  }

  // Reset and copy from channel 2 (default)
  std::fill(monoData.begin(), monoData.end(), 0.0f);
  monoBuffer.CopyChannelFrom(srcBuffer, 2);

  // Verify data from channel 2
  for (unsigned i = 0; i < srcSamples; ++i) {
    GOAssert(
      monoData[i] == srcData[i * srcChannels + 2],
      std::format(
        "Sample {} should be {} (got: {})",
        i,
        srcData[i * srcChannels + 2],
        monoData[i]));
  }
}

void GOTestSoundBufferMutableMono::TestCopyChannelTo() {
  const unsigned dstChannels = 3;
  const unsigned dstSamples = 4;
  const unsigned dstTotalUnits = dstChannels * dstSamples;

  // Create mono source buffer
  std::vector<GOSoundBuffer::SoundUnit> monoData(dstSamples);
  for (unsigned i = 0; i < dstSamples; ++i) {
    monoData[i] = static_cast<float>((i + 1) * 10);
  }

  GOSoundBufferMutableMono monoBuffer(monoData.data(), dstSamples);

  // Create multi-channel destination buffer
  std::vector<GOSoundBuffer::SoundUnit> dstData(dstTotalUnits, 0.0f);
  GOSoundBufferMutable dstBuffer(dstData.data(), dstChannels, dstSamples);

  // Copy to channel 0
  monoBuffer.CopyChannelTo(dstBuffer, 0);

  // Verify data in channel 0
  for (unsigned i = 0; i < dstSamples; ++i) {
    GOAssert(
      dstData[i * dstChannels] == monoData[i],
      std::format(
        "Channel 0, sample {} should be {} (got: {})",
        i,
        monoData[i],
        dstData[i * dstChannels]));
  }

  // Copy to channel 1
  monoBuffer.CopyChannelTo(dstBuffer, 1);

  // Verify data in channel 1
  for (unsigned i = 0; i < dstSamples; ++i) {
    GOAssert(
      dstData[i * dstChannels + 1] == monoData[i],
      std::format(
        "Channel 1, sample {} should be {} (got: {})",
        i,
        monoData[i],
        dstData[i * dstChannels + 1]));
  }

  // Copy to channel 2 (default)
  monoBuffer.CopyChannelTo(dstBuffer, 2);

  // Verify data in channel 2
  for (unsigned i = 0; i < dstSamples; ++i) {
    GOAssert(
      dstData[i * dstChannels + 2] == monoData[i],
      std::format(
        "Channel 2, sample {} should be {} (got: {})",
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

  // Buffer with zero samples
  float dummyData[1];
  GOSoundBufferMutableMono zeroSamplesBuffer(dummyData, 0);
  GOAssert(
    !zeroSamplesBuffer.isValid(), "Buffer with 0 samples should be invalid");

  // Valid buffer with one sample
  float singleSample = 1.0f;
  GOSoundBufferMutableMono singleBuffer(&singleSample, 1);
  GOAssert(singleBuffer.isValid(), "Buffer with 1 sample should be valid");

  GOAssert(
    singleBuffer.GetNUnits() == 1,
    std::format(
      "Buffer with 1 sample should have 1 total unit (got: {})",
      singleBuffer.GetNUnits()));
}

void GOTestSoundBufferMutableMono::TestEdgeCases() {
  // Large mono buffer
  const unsigned largeSamples = 10000;

  std::vector<GOSoundBuffer::SoundUnit> largeData(largeSamples);
  std::fill(largeData.begin(), largeData.end(), 0.5f);

  GOSoundBufferMutableMono largeBuffer(largeData.data(), largeSamples);

  GOAssert(largeBuffer.isValid(), "Large mono buffer should be valid");

  // GetSubBuffer at the very end
  GOSoundBufferMutableMono lastSample
    = largeBuffer.GetSubBuffer(largeSamples - 1, 1);
  GOAssert(lastSample.isValid(), "GetSubBuffer at last sample should be valid");

  GOAssert(
    lastSample.GetNSamples() == 1,
    std::format(
      "Last sample GetSubBuffer should have 1 sample (got: {})",
      lastSample.GetNSamples()));

  // GetSubBuffer of entire length
  GOSoundBufferMutableMono fullBuffer
    = largeBuffer.GetSubBuffer(0, largeSamples);
  GOAssert(fullBuffer.isValid(), "GetSubBuffer of full length should be valid");

  GOAssert(
    fullBuffer.GetData() == largeData.data(),
    "GetSubBuffer of full length should point to same data");

  // Test CopyChannelFrom with stereo buffer
  const unsigned stereoChannels = 2;
  const unsigned stereoDataSize = stereoChannels * largeSamples;
  std::vector<GOSoundBuffer::SoundUnit> stereoData(stereoDataSize);
  for (unsigned i = 0; i < stereoDataSize; ++i) {
    stereoData[i] = static_cast<float>(i % 100);
  }

  GOSoundBuffer stereoBuffer(stereoData.data(), stereoChannels, largeSamples);

  // Copy from left channel
  std::vector<GOSoundBuffer::SoundUnit> monoResult(largeSamples, 0.0f);
  GOSoundBufferMutableMono monoResultBuffer(monoResult.data(), largeSamples);
  monoResultBuffer.CopyChannelFrom(stereoBuffer, 0);

  // Verify left channel data
  for (unsigned i = 0; i < largeSamples; ++i) {
    GOAssert(
      monoResult[i] == stereoData[i * stereoChannels],
      std::format("Left channel sample {} mismatch", i));
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