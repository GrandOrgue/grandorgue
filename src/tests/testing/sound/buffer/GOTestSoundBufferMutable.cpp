/*
 * Copyright 2024-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundBufferMutable.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <format>
#include <iostream>
#include <numeric>
#include <vector>

#include "sound/buffer/GOSoundBufferMutable.h"

const std::string GOTestSoundBufferMutable::TEST_NAME
  = "GOTestSoundBufferMutable";

void GOTestSoundBufferMutable::TestInheritanceAndMutableAccess() {
  const unsigned nChannels = 2;
  const unsigned nSamples = 3;
  const unsigned totalUnits = nChannels * nSamples;

  std::vector<GOSoundBuffer::SoundUnit> data(totalUnits, 1.0f);
  GOSoundBufferMutable buffer(data.data(), nChannels, nSamples);

  GOAssert(buffer.isValid(), "Mutable buffer should be valid");

  GOAssert(
    buffer.GetNChannels() == nChannels,
    std::format(
      "Mutable buffer should have {} channels (got: {})",
      nChannels,
      buffer.GetNChannels()));

  GOAssert(
    buffer.GetNSamples() == nSamples,
    std::format(
      "Mutable buffer should have {} samples (got: {})",
      nSamples,
      buffer.GetNSamples()));

  // Check that we can modify data
  buffer.GetData()[0] = 42.0f;
  GOAssert(
    data[0] == 42.0f,
    std::format("Data should be mutable (expected: 42.0, got: {})", data[0]));

  // Reset
  data[0] = 1.0f;
}

void GOTestSoundBufferMutable::TestFillWithSilence() {
  const unsigned nChannels = 2;
  const unsigned nSamples = 4;
  const unsigned totalUnits = nChannels * nSamples;

  std::vector<GOSoundBuffer::SoundUnit> data(totalUnits);
  std::fill(data.begin(), data.end(), 1.0f);

  GOSoundBufferMutable buffer(data.data(), nChannels, nSamples);
  buffer.FillWithSilence();

  for (unsigned i = 0; i < totalUnits; ++i) {
    GOAssert(
      data[i] == 0.0f,
      std::format(
        "All units should be 0.0 after FillWithSilence (unit {}: {})",
        i,
        data[i]));
  }
}

void GOTestSoundBufferMutable::TestCopyFrom() {
  const unsigned nChannels = 2;
  const unsigned nSamples = 3;
  const unsigned totalUnits = nChannels * nSamples;

  std::vector<GOSoundBuffer::SoundUnit> sourceData(totalUnits);
  std::vector<GOSoundBuffer::SoundUnit> destData(totalUnits, 0.0f);

  for (unsigned i = 0; i < totalUnits; ++i) {
    sourceData[i] = static_cast<float>(i + 1);
  }

  GOSoundBuffer sourceBuffer(sourceData.data(), nChannels, nSamples);
  GOSoundBufferMutable destBuffer(destData.data(), nChannels, nSamples);

  destBuffer.CopyFrom(sourceBuffer);

  for (unsigned i = 0; i < totalUnits; ++i) {
    GOAssert(
      destData[i] == sourceData[i],
      std::format(
        "Dest unit {} should equal source unit {} (dest: {}, source: {})",
        i,
        i,
        destData[i],
        sourceData[i]));
  }
}

void GOTestSoundBufferMutable::TestAddFrom() {
  const unsigned nChannels = 2;
  const unsigned nSamples = 2;
  const unsigned totalUnits = nChannels * nSamples;

  std::vector<GOSoundBuffer::SoundUnit> dataA = {1.0f, 2.0f, 3.0f, 4.0f};
  std::vector<GOSoundBuffer::SoundUnit> dataB = {0.5f, 1.0f, 1.5f, 2.0f};
  std::vector<GOSoundBuffer::SoundUnit> result(totalUnits, 0.0f);

  GOSoundBuffer bufferA(dataA.data(), nChannels, nSamples);
  GOSoundBuffer bufferB(dataB.data(), nChannels, nSamples);
  GOSoundBufferMutable mutableBuffer(result.data(), nChannels, nSamples);

  // First copy A
  mutableBuffer.CopyFrom(bufferA);

  // Then add B
  mutableBuffer.AddFrom(bufferB);

  std::vector<GOSoundBuffer::SoundUnit> expected = {1.5f, 3.0f, 4.5f, 6.0f};

  for (unsigned i = 0; i < totalUnits; ++i) {
    GOAssert(
      result[i] == expected[i],
      std::format("Unit {} should be {} (got: {})", i, expected[i], result[i]));
  }
}

void GOTestSoundBufferMutable::TestAddFromWithCoefficient() {
  const unsigned nChannels = 2;
  const unsigned nSamples = 2;
  const unsigned totalUnits = nChannels * nSamples;

  std::vector<GOSoundBuffer::SoundUnit> dataA = {1.0f, 2.0f, 3.0f, 4.0f};
  std::vector<GOSoundBuffer::SoundUnit> dataB = {0.5f, 1.0f, 1.5f, 2.0f};
  std::vector<GOSoundBuffer::SoundUnit> result(totalUnits, 0.0f);

  GOSoundBuffer bufferA(dataA.data(), nChannels, nSamples);
  GOSoundBuffer bufferB(dataB.data(), nChannels, nSamples);
  GOSoundBufferMutable mutableBuffer(result.data(), nChannels, nSamples);

  // First copy A
  mutableBuffer.CopyFrom(bufferA);

  // Then add B with coefficient 2.0
  const float coefficient = 2.0f;
  mutableBuffer.AddFrom(bufferB, coefficient);

  std::vector<GOSoundBuffer::SoundUnit> expected = {2.0f, 4.0f, 6.0f, 8.0f};

  for (unsigned i = 0; i < totalUnits; ++i) {
    GOAssert(
      result[i] == expected[i],
      std::format("Unit {} should be {} (got: {})", i, expected[i], result[i]));
  }
}

void GOTestSoundBufferMutable::TestMutableGetSubBuffer() {
  const unsigned nChannels = 2;
  const unsigned nSamples = 4;
  const unsigned totalUnits = nChannels * nSamples;

  std::vector<GOSoundBuffer::SoundUnit> data(totalUnits);
  std::iota(data.begin(), data.end(), 1.0f);

  GOSoundBufferMutable buffer(data.data(), nChannels, nSamples);

  // Create mutable sub-buffer
  GOSoundBufferMutable subBuffer = buffer.GetSubBuffer(1, 2);

  GOAssert(subBuffer.isValid(), "Mutable SubBuffer should be valid");

  GOAssert(
    subBuffer.GetNSamples() == 2,
    std::format(
      "Mutable SubBuffer should have 2 samples (got: {})",
      subBuffer.GetNSamples()));

  // Modify data through sub-buffer
  subBuffer.FillWithSilence();

  // Check that only sub-buffer data changed
  GOAssert(
    data[0] == 1.0f && data[1] == 2.0f, "First sample should remain unchanged");

  GOAssert(
    data[2] == 0.0f && data[3] == 0.0f, "Second sample should be zeroed");

  GOAssert(data[4] == 0.0f && data[5] == 0.0f, "Third sample should be zeroed");

  GOAssert(
    data[6] == 7.0f && data[7] == 8.0f,
    "Fourth sample should remain unchanged");
}

void GOTestSoundBufferMutable::TestCompatibilityChecks() {
  const unsigned nChannels = 2;
  const unsigned nSamples = 3;

  std::vector<GOSoundBuffer::SoundUnit> data1(nChannels * nSamples, 1.0f);
  std::vector<GOSoundBuffer::SoundUnit> data2(nChannels * (nSamples + 1), 1.0f);
  std::vector<GOSoundBuffer::SoundUnit> data3((nChannels + 1) * nSamples, 1.0f);

  GOSoundBuffer buffer1(data1.data(), nChannels, nSamples);
  GOSoundBuffer buffer2(data2.data(), nChannels, nSamples + 1);
  GOSoundBuffer buffer3(data3.data(), nChannels + 1, nSamples);

  std::vector<GOSoundBuffer::SoundUnit> mutableData(nChannels * nSamples, 0.0f);
  GOSoundBufferMutable mutableBuffer(mutableData.data(), nChannels, nSamples);

  // Compatible buffers should work
  mutableBuffer.CopyFrom(buffer1);

  // Check that data was copied
  for (unsigned i = 0; i < nChannels * nSamples; ++i) {
    GOAssert(
      mutableData[i] == 1.0f,
      std::format(
        "Unit {} should be 1.0 after copy (got: {})", i, mutableData[i]));
  }

  // Incompatible buffers should cause assert in debug builds
  // (In release builds this leads to undefined behavior)
  // mutableBuffer.CopyFrom(buffer2); // Different sample count
  // mutableBuffer.CopyFrom(buffer3); // Different channel count
}

void GOTestSoundBufferMutable::TestComplexOperations() {
  const unsigned nChannels = 2;
  const unsigned nSamples = 3;
  const unsigned totalUnits = nChannels * nSamples;

  std::vector<GOSoundBuffer::SoundUnit> sourceA(totalUnits);
  std::vector<GOSoundBuffer::SoundUnit> sourceB(totalUnits);
  std::vector<GOSoundBuffer::SoundUnit> result(totalUnits, 0.0f);

  for (unsigned i = 0; i < totalUnits; ++i) {
    sourceA[i] = static_cast<float>(i + 1);
    sourceB[i] = static_cast<float>(i + 1) * 0.1f;
  }

  GOSoundBuffer bufferA(sourceA.data(), nChannels, nSamples);
  GOSoundBuffer bufferB(sourceB.data(), nChannels, nSamples);
  GOSoundBufferMutable mutableBuffer(result.data(), nChannels, nSamples);

  // Complex operation: result = 0.5*A + 2.0*B
  mutableBuffer.CopyFrom(bufferA);
  mutableBuffer.AddFrom(bufferA, -0.5f); // result = 0.5*A
  mutableBuffer.AddFrom(bufferB, 2.0f);  // result = 0.5*A + 2.0*B

  for (unsigned i = 0; i < totalUnits; ++i) {
    float expected = 0.5f * sourceA[i] + 2.0f * sourceB[i];
    const float epsilon = 1e-6f;
    float diff = std::abs(result[i] - expected);

    GOAssert(
      diff < epsilon,
      std::format(
        "Unit {} should be {} (got: {}, diff: {})",
        i,
        expected,
        result[i],
        diff));
  }
}

void GOTestSoundBufferMutable::TestCopyChannelFrom() {
  const unsigned srcChannels = 3;
  const unsigned dstChannels = 2;
  const unsigned nSamples = 4;

  // Source buffer with 3 channels
  std::vector<GOSoundBuffer::SoundUnit> srcData = {// Sample 0: ch0, ch1, ch2
                                                   1.0f,
                                                   2.0f,
                                                   3.0f,
                                                   // Sample 1
                                                   4.0f,
                                                   5.0f,
                                                   6.0f,
                                                   // Sample 2
                                                   7.0f,
                                                   8.0f,
                                                   9.0f,
                                                   // Sample 3
                                                   10.0f,
                                                   11.0f,
                                                   12.0f};

  // Destination buffer with 2 channels (initialized with zeros)
  std::vector<GOSoundBuffer::SoundUnit> dstData(srcChannels * nSamples, 0.0f);

  GOSoundBuffer srcBuffer(srcData.data(), srcChannels, nSamples);
  GOSoundBufferMutable dstBuffer(dstData.data(), dstChannels, nSamples);

  // Copy channel 1 from source to channel 0 of destination
  dstBuffer.CopyChannelFrom(srcBuffer, 1, 0);

  // Verify that channel 0 of destination now contains data from channel 1 of
  // source and channel 1 of destination remains zero
  for (unsigned i = 0; i < nSamples; ++i) {
    GOAssert(
      dstData[i * dstChannels] == srcData[i * srcChannels + 1],
      std::format(
        "Sample {} channel 0 should be {} (got: {})",
        i,
        srcData[i * srcChannels + 1],
        dstData[i * dstChannels]));

    GOAssert(
      dstData[i * dstChannels + 1] == 0.0f,
      std::format(
        "Sample {} channel 1 should still be 0.0 (got: {})",
        i,
        dstData[i * dstChannels + 1]));
  }

  // Copy channel 2 from source to channel 1 of destination
  dstBuffer.CopyChannelFrom(srcBuffer, 2, 1);

  // Verify both channels
  for (unsigned i = 0; i < nSamples; ++i) {
    GOAssert(
      dstData[i * dstChannels] == srcData[i * srcChannels + 1],
      std::format(
        "Sample {} channel 0 should be {} (got: {})",
        i,
        srcData[i * srcChannels + 1],
        dstData[i * dstChannels]));

    GOAssert(
      dstData[i * dstChannels + 1] == srcData[i * srcChannels + 2],
      std::format(
        "Sample {} channel 1 should be {} (got: {})",
        i,
        srcData[i * srcChannels + 2],
        dstData[i * dstChannels + 1]));
  }
}

void GOTestSoundBufferMutable::TestAddChannelFrom() {
  const unsigned srcChannels = 2;
  const unsigned dstChannels = 2;
  const unsigned nSamples = 3;

  // Source buffer
  std::vector<GOSoundBuffer::SoundUnit> srcData = {// Sample 0: ch0, ch1
                                                   1.0f,
                                                   2.0f,
                                                   // Sample 1
                                                   3.0f,
                                                   4.0f,
                                                   // Sample 2
                                                   5.0f,
                                                   6.0f};

  // Destination buffer with initial values
  std::vector<GOSoundBuffer::SoundUnit> dstData = {// Sample 0
                                                   10.0f,
                                                   20.0f,
                                                   // Sample 1
                                                   30.0f,
                                                   40.0f,
                                                   // Sample 2
                                                   50.0f,
                                                   60.0f};

  GOSoundBuffer srcBuffer(srcData.data(), srcChannels, nSamples);
  GOSoundBufferMutable dstBuffer(dstData.data(), dstChannels, nSamples);

  // Add channel 0 from source to channel 0 of destination
  dstBuffer.AddChannelFrom(srcBuffer, 0, 0);

  // Verify results
  std::vector<GOSoundBuffer::SoundUnit> expectedAfterFirst = {
    11.0f,
    20.0f, // 10 + 1, 20
    33.0f,
    40.0f, // 30 + 3, 40
    55.0f,
    60.0f // 50 + 5, 60
  };

  for (unsigned i = 0; i < dstData.size(); ++i) {
    GOAssert(
      dstData[i] == expectedAfterFirst[i],
      std::format(
        "Unit {} should be {} after AddChannelFrom(ch0->ch0) (got: {})",
        i,
        expectedAfterFirst[i],
        dstData[i]));
  }

  // Add channel 1 from source to channel 1 of destination
  dstBuffer.AddChannelFrom(srcBuffer, 1, 1);

  // Verify final results
  std::vector<GOSoundBuffer::SoundUnit> expectedAfterSecond = {
    11.0f,
    22.0f, // 20 + 2
    33.0f,
    44.0f, // 40 + 4
    55.0f,
    66.0f // 60 + 6
  };

  for (unsigned i = 0; i < dstData.size(); ++i) {
    GOAssert(
      dstData[i] == expectedAfterSecond[i],
      std::format(
        "Unit {} should be {} after AddChannelFrom(ch1->ch1) (got: {})",
        i,
        expectedAfterSecond[i],
        dstData[i]));
  }
}

void GOTestSoundBufferMutable::TestAddChannelFromWithCoefficient() {
  const unsigned srcChannels = 2;
  const unsigned dstChannels = 2;
  const unsigned nSamples = 3;

  // Source buffer
  std::vector<GOSoundBuffer::SoundUnit> srcData
    = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f};

  // Destination buffer with initial values
  std::vector<GOSoundBuffer::SoundUnit> dstData
    = {10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f};

  GOSoundBuffer srcBuffer(srcData.data(), srcChannels, nSamples);
  GOSoundBufferMutable dstBuffer(dstData.data(), dstChannels, nSamples);

  const float coefficient = 0.5f;

  // Add channel 0 from source to channel 0 of destination with coefficient 0.5
  dstBuffer.AddChannelFrom(srcBuffer, 0, 0, coefficient);

  // Verify results
  std::vector<GOSoundBuffer::SoundUnit> expected = {
    10.5f,
    20.0f, // 10 + 1*0.5, 20
    31.5f,
    40.0f, // 30 + 3*0.5, 40
    52.5f,
    60.0f // 50 + 5*0.5, 60
  };

  for (unsigned i = 0; i < dstData.size(); ++i) {
    const float epsilon = 1e-6f;
    float diff = std::abs(dstData[i] - expected[i]);

    GOAssert(
      diff < epsilon,
      std::format(
        "Unit {} should be {} (got: {}, diff: {})",
        i,
        expected[i],
        dstData[i],
        diff));
  }

  // Test with negative coefficient
  const float negativeCoefficient = -1.0f;
  std::vector<GOSoundBuffer::SoundUnit> dstData2
    = dstData; // Copy current state

  GOSoundBufferMutable dstBuffer2(dstData2.data(), dstChannels, nSamples);
  dstBuffer2.AddChannelFrom(srcBuffer, 1, 1, negativeCoefficient);

  // Expect: channel 1 decreased by values from channel 1 of source
  std::vector<GOSoundBuffer::SoundUnit> expected2 = {
    10.5f,
    18.0f, // 20.0 - 2.0
    31.5f,
    36.0f, // 40.0 - 4.0
    52.5f,
    54.0f // 60.0 - 6.0
  };

  for (unsigned i = 0; i < dstData2.size(); ++i) {
    const float epsilon = 1e-6f;
    float diff = std::abs(dstData2[i] - expected2[i]);

    GOAssert(
      diff < epsilon,
      std::format(
        "Unit {} should be {} with negative coefficient (got: {}, diff: {})",
        i,
        expected2[i],
        dstData2[i],
        diff));
  }
}

void GOTestSoundBufferMutable::TestCrossChannelOperations() {
  const unsigned nChannels = 3;
  const unsigned nSamples = 4;

  // Source buffer
  std::vector<GOSoundBuffer::SoundUnit> srcData = {
    1.0f,
    10.0f,
    100.0f,
    2.0f,
    20.0f,
    200.0f,
    3.0f,
    30.0f,
    300.0f,
    4.0f,
    40.0f,
    400.0f};

  // Destination buffer
  std::vector<GOSoundBuffer::SoundUnit> dstData(nChannels * nSamples, 0.0f);

  GOSoundBuffer srcBuffer(srcData.data(), nChannels, nSamples);
  GOSoundBufferMutable dstBuffer(dstData.data(), nChannels, nSamples);

  // Complex operations with different channels:

  // 1. Copy channel 0 to channel 1
  dstBuffer.CopyChannelFrom(srcBuffer, 0, 1);

  // 2. Add channel 1 (multiplied by 2) to channel 2
  dstBuffer.AddChannelFrom(srcBuffer, 1, 2, 2.0f);

  // 3. Add channel 2 (multiplied by 0.5) to channel 0
  dstBuffer.AddChannelFrom(srcBuffer, 2, 0, 0.5f);

  // Verify results
  for (unsigned i = 0; i < nSamples; ++i) {
    const unsigned base = i * nChannels;

    // Channel 0: 0.5 * source channel 2
    float expectedCh0 = srcData[base + 2] * 0.5f;
    GOAssert(
      dstData[base] == expectedCh0,
      std::format(
        "Sample {} channel 0 should be {} (got: {})",
        i,
        expectedCh0,
        dstData[base]));

    // Channel 1: source channel 0
    float expectedCh1 = srcData[base];
    GOAssert(
      dstData[base + 1] == expectedCh1,
      std::format(
        "Sample {} channel 1 should be {} (got: {})",
        i,
        expectedCh1,
        dstData[base + 1]));

    // Channel 2: 2.0 * source channel 1
    float expectedCh2 = srcData[base + 1] * 2.0f;
    GOAssert(
      dstData[base + 2] == expectedCh2,
      std::format(
        "Sample {} channel 2 should be {} (got: {})",
        i,
        expectedCh2,
        dstData[base + 2]));
  }
}

void GOTestSoundBufferMutable::TestChannelOperationsWithSubBuffers() {
  const unsigned nChannels = 2;
  const unsigned nSamples = 6;

  // Main buffer
  std::vector<GOSoundBuffer::SoundUnit> mainData = {
    1.0f,
    101.0f,
    2.0f,
    102.0f,
    3.0f,
    103.0f,
    4.0f,
    104.0f,
    5.0f,
    105.0f,
    6.0f,
    106.0f};

  // Create mutable buffer
  GOSoundBufferMutable mainBuffer(mainData.data(), nChannels, nSamples);

  // Create sub-buffer (samples 2-4, i.e., 3 samples)
  GOSoundBufferMutable subBuffer = mainBuffer.GetSubBuffer(2, 3);

  // Create separate source for copying
  std::vector<GOSoundBuffer::SoundUnit> sourceData
    = {10.0f, 20.0f, 11.0f, 21.0f, 12.0f, 22.0f};

  GOSoundBuffer sourceBuffer(sourceData.data(), nChannels, 3);

  // Copy from source to sub-buffer, channel 0 to channel 1
  subBuffer.CopyChannelFrom(sourceBuffer, 0, 1);

  // Verify that only sub-buffer changed
  std::vector<GOSoundBuffer::SoundUnit> expectedMainData = {
    1.0f,
    101.0f, // Unchanged
    2.0f,
    102.0f, // Unchanged
    3.0f,
    10.0f, // Changed: channel 1 = 10 (was 103)
    4.0f,
    11.0f, // Changed: channel 1 = 11 (was 104)
    5.0f,
    12.0f, // Changed: channel 1 = 12 (was 105)
    6.0f,
    106.0f // Unchanged
  };

  for (unsigned i = 0; i < mainData.size(); ++i) {
    GOAssert(
      mainData[i] == expectedMainData[i],
      std::format(
        "Main buffer unit {} should be {} (got: {})",
        i,
        expectedMainData[i],
        mainData[i]));
  }
}

void GOTestSoundBufferMutable::TestInvalidChannelIndices() {
  // This test verifies that using invalid channel indices
  // triggers assert in debug builds

  const unsigned srcChannels = 2;
  const unsigned dstChannels = 2;
  const unsigned nSamples = 3;

  std::vector<GOSoundBuffer::SoundUnit> srcData(srcChannels * nSamples, 1.0f);
  std::vector<GOSoundBuffer::SoundUnit> dstData(dstChannels * nSamples, 0.0f);

  GOSoundBuffer srcBuffer(srcData.data(), srcChannels, nSamples);
  GOSoundBufferMutable dstBuffer(dstData.data(), dstChannels, nSamples);

  // These operations should work correctly
  dstBuffer.CopyChannelFrom(srcBuffer, 0, 0); // Valid indices
  dstBuffer.CopyChannelFrom(srcBuffer, 1, 1); // Valid indices

  // The following operations should trigger assert in debug mode
  // (commented out as they would cause test failure in debug)
  /*
  // Invalid source index
  // dstBuffer.CopyChannelFrom(srcBuffer, 2, 0);  // srcChannel=2, but only 0,1
  exist

  // Invalid destination index
  // dstBuffer.CopyChannelFrom(srcBuffer, 0, 2);  // dstChannel=2, but only 0,1
  exist

  // Both invalid
  // dstBuffer.CopyChannelFrom(srcBuffer, 2, 2);
  */

  // Instead, verify that valid operations completed successfully
  for (unsigned i = 0; i < nSamples; ++i) {
    GOAssert(
      dstData[i * dstChannels] == 1.0f,
      std::format("Sample {} channel 0 should be 1.0", i));
    GOAssert(
      dstData[i * dstChannels + 1] == 1.0f,
      std::format("Sample {} channel 1 should be 1.0", i));
  }
}

void GOTestSoundBufferMutable::TestSingleChannelBuffer() {
  // Test operations with single-channel buffers

  const unsigned monoChannels = 1;
  const unsigned stereoChannels = 2;
  const unsigned nSamples = 4;

  // Mono source
  std::vector<GOSoundBuffer::SoundUnit> monoData = {1.0f, 2.0f, 3.0f, 4.0f};

  // Stereo destination
  std::vector<GOSoundBuffer::SoundUnit> stereoData(
    stereoChannels * nSamples, 0.0f);

  GOSoundBuffer monoBuffer(monoData.data(), monoChannels, nSamples);
  GOSoundBufferMutable stereoBuffer(
    stereoData.data(), stereoChannels, nSamples);

  // Copy from mono to left channel of stereo
  stereoBuffer.CopyChannelFrom(monoBuffer, 0, 0);

  // Copy from mono to right channel of stereo with coefficient
  stereoBuffer.AddChannelFrom(monoBuffer, 0, 1, 0.5f);

  // Verify results
  for (unsigned i = 0; i < nSamples; ++i) {
    GOAssert(
      stereoData[i * stereoChannels] == monoData[i],
      std::format(
        "Left channel sample {} should be {} (got: {})",
        i,
        monoData[i],
        stereoData[i * stereoChannels]));

    GOAssert(
      stereoData[i * stereoChannels + 1] == monoData[i] * 0.5f,
      std::format(
        "Right channel sample {} should be {} (got: {})",
        i,
        monoData[i] * 0.5f,
        stereoData[i * stereoChannels + 1]));
  }
}

void GOTestSoundBufferMutable::run() {
  TestInheritanceAndMutableAccess();
  TestFillWithSilence();
  TestCopyFrom();
  TestAddFrom();
  TestAddFromWithCoefficient();
  TestMutableGetSubBuffer();
  TestCompatibilityChecks();
  TestComplexOperations();
  TestCopyChannelFrom();
  TestAddChannelFrom();
  TestAddChannelFromWithCoefficient();
  TestCrossChannelOperations();
  TestChannelOperationsWithSubBuffers();
  TestInvalidChannelIndices();
  TestSingleChannelBuffer();
}
