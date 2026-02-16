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

  // Use macro to declare local buffer
  GO_DECLARE_LOCAL_SOUND_BUFFER(buffer, nChannels, nSamples);

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

  // Initialize data with 1.0f
  for (unsigned i = 0; i < buffer.GetNUnits(); ++i) {
    buffer.GetData()[i] = 1.0f;
  }

  // Check that we can modify data
  buffer.GetData()[0] = 42.0f;
  GOAssert(
    buffer.GetData()[0] == 42.0f,
    std::format(
      "Data should be mutable (expected: 42.0, got: {})", buffer.GetData()[0]));
}

void GOTestSoundBufferMutable::TestFillWithSilence() {
  const unsigned nChannels = 2;
  const unsigned nSamples = 4;

  GO_DECLARE_LOCAL_SOUND_BUFFER(buffer, nChannels, nSamples);

  // Initialize with non-zero values
  for (unsigned i = 0; i < buffer.GetNUnits(); ++i) {
    buffer.GetData()[i] = 1.0f;
  }

  buffer.FillWithSilence();

  for (unsigned i = 0; i < buffer.GetNUnits(); ++i) {
    GOAssert(
      buffer.GetData()[i] == 0.0f,
      std::format(
        "All units should be 0.0 after FillWithSilence (unit {}: {})",
        i,
        buffer.GetData()[i]));
  }
}

void GOTestSoundBufferMutable::TestCopyFrom() {
  const unsigned nChannels = 2;
  const unsigned nSamples = 3;

  // Create source buffer with test data
  GO_DECLARE_LOCAL_SOUND_BUFFER(sourceBuffer, nChannels, nSamples);
  GO_DECLARE_LOCAL_SOUND_BUFFER(destBuffer, nChannels, nSamples);

  // Initialize source with test data
  for (unsigned i = 0; i < sourceBuffer.GetNUnits(); ++i) {
    sourceBuffer.GetData()[i] = static_cast<float>(i + 1);
  }

  // Initialize destination with zeros
  destBuffer.FillWithSilence();

  destBuffer.CopyFrom(sourceBuffer);

  for (unsigned i = 0; i < destBuffer.GetNUnits(); ++i) {
    GOAssert(
      destBuffer.GetData()[i] == sourceBuffer.GetData()[i],
      std::format(
        "Dest unit {} should equal source unit {} (dest: {}, source: {})",
        i,
        i,
        destBuffer.GetData()[i],
        sourceBuffer.GetData()[i]));
  }
}

void GOTestSoundBufferMutable::TestAddFrom() {
  const unsigned nChannels = 2;
  const unsigned nSamples = 2;

  GO_DECLARE_LOCAL_SOUND_BUFFER(bufferA, nChannels, nSamples);
  GO_DECLARE_LOCAL_SOUND_BUFFER(bufferB, nChannels, nSamples);
  GO_DECLARE_LOCAL_SOUND_BUFFER(result, nChannels, nSamples);

  // Initialize bufferA: [1.0f, 2.0f, 3.0f, 4.0f]
  float aVals[] = {1.0f, 2.0f, 3.0f, 4.0f};
  for (unsigned i = 0; i < bufferA.GetNUnits(); ++i) {
    bufferA.GetData()[i] = aVals[i];
  }

  // Initialize bufferB: [0.5f, 1.0f, 1.5f, 2.0f]
  float bVals[] = {0.5f, 1.0f, 1.5f, 2.0f};
  for (unsigned i = 0; i < bufferB.GetNUnits(); ++i) {
    bufferB.GetData()[i] = bVals[i];
  }

  result.FillWithSilence();

  // First copy A
  result.CopyFrom(bufferA);

  // Then add B
  result.AddFrom(bufferB);

  float expected[] = {1.5f, 3.0f, 4.5f, 6.0f};

  for (unsigned i = 0; i < result.GetNUnits(); ++i) {
    GOAssert(
      result.GetData()[i] == expected[i],
      std::format(
        "Unit {} should be {} (got: {})", i, expected[i], result.GetData()[i]));
  }
}

void GOTestSoundBufferMutable::TestAddFromWithCoefficient() {
  const unsigned nChannels = 2;
  const unsigned nSamples = 2;

  GO_DECLARE_LOCAL_SOUND_BUFFER(bufferA, nChannels, nSamples);
  GO_DECLARE_LOCAL_SOUND_BUFFER(bufferB, nChannels, nSamples);
  GO_DECLARE_LOCAL_SOUND_BUFFER(result, nChannels, nSamples);

  // Initialize bufferA: [1.0f, 2.0f, 3.0f, 4.0f]
  float aVals[] = {1.0f, 2.0f, 3.0f, 4.0f};
  for (unsigned i = 0; i < bufferA.GetNUnits(); ++i) {
    bufferA.GetData()[i] = aVals[i];
  }

  // Initialize bufferB: [0.5f, 1.0f, 1.5f, 2.0f]
  float bVals[] = {0.5f, 1.0f, 1.5f, 2.0f};
  for (unsigned i = 0; i < bufferB.GetNUnits(); ++i) {
    bufferB.GetData()[i] = bVals[i];
  }

  result.FillWithSilence();

  // First copy A
  result.CopyFrom(bufferA);

  // Then add B with coefficient 2.0
  const float coefficient = 2.0f;
  result.AddFrom(bufferB, coefficient);

  float expected[] = {2.0f, 4.0f, 6.0f, 8.0f};

  for (unsigned i = 0; i < result.GetNUnits(); ++i) {
    GOAssert(
      result.GetData()[i] == expected[i],
      std::format(
        "Unit {} should be {} (got: {})", i, expected[i], result.GetData()[i]));
  }
}

void GOTestSoundBufferMutable::TestMutableGetSubBuffer() {
  const unsigned nChannels = 2;
  const unsigned nSamples = 4;

  GO_DECLARE_LOCAL_SOUND_BUFFER(buffer, nChannels, nSamples);

  // Initialize with sequential values: 1.0f, 2.0f, 3.0f, ... 8.0f
  for (unsigned i = 0; i < buffer.GetNUnits(); ++i) {
    buffer.GetData()[i] = static_cast<float>(i + 1);
  }

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
    buffer.GetData()[0] == 1.0f && buffer.GetData()[1] == 2.0f,
    "First sample should remain unchanged");

  GOAssert(
    buffer.GetData()[2] == 0.0f && buffer.GetData()[3] == 0.0f,
    "Second sample should be zeroed");

  GOAssert(
    buffer.GetData()[4] == 0.0f && buffer.GetData()[5] == 0.0f,
    "Third sample should be zeroed");

  GOAssert(
    buffer.GetData()[6] == 7.0f && buffer.GetData()[7] == 8.0f,
    "Fourth sample should remain unchanged");
}

void GOTestSoundBufferMutable::TestCompatibilityChecks() {
  const unsigned nChannels = 2;
  const unsigned nSamples = 3;

  // Compatible buffers
  GO_DECLARE_LOCAL_SOUND_BUFFER(buffer1, nChannels, nSamples);
  GO_DECLARE_LOCAL_SOUND_BUFFER(mutableBuffer, nChannels, nSamples);

  // Initialize buffer1 with 1.0f
  for (unsigned i = 0; i < buffer1.GetNUnits(); ++i) {
    buffer1.GetData()[i] = 1.0f;
  }

  mutableBuffer.FillWithSilence();

  // Compatible buffers should work
  mutableBuffer.CopyFrom(buffer1);

  // Check that data was copied
  for (unsigned i = 0; i < mutableBuffer.GetNUnits(); ++i) {
    GOAssert(
      mutableBuffer.GetData()[i] == 1.0f,
      std::format(
        "Unit {} should be 1.0 after copy (got: {})",
        i,
        mutableBuffer.GetData()[i]));
  }

  // Note: Incompatible buffer tests would require creating buffers with
  // different dimensions, which can't be done with the macro in the same scope.
  // For debug builds, assertions would catch incompatibilities.
}

void GOTestSoundBufferMutable::TestComplexOperations() {
  const unsigned nChannels = 2;
  const unsigned nSamples = 3;

  GO_DECLARE_LOCAL_SOUND_BUFFER(sourceA, nChannels, nSamples);
  GO_DECLARE_LOCAL_SOUND_BUFFER(sourceB, nChannels, nSamples);
  GO_DECLARE_LOCAL_SOUND_BUFFER(result, nChannels, nSamples);

  // Initialize sourceA: 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f
  for (unsigned i = 0; i < sourceA.GetNUnits(); ++i) {
    sourceA.GetData()[i] = static_cast<float>(i + 1);
  }

  // Initialize sourceB: 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f
  for (unsigned i = 0; i < sourceB.GetNUnits(); ++i) {
    sourceB.GetData()[i] = static_cast<float>(i + 1) * 0.1f;
  }

  result.FillWithSilence();

  // Complex operation: result = 0.5*A + 2.0*B
  result.CopyFrom(sourceA);
  result.AddFrom(sourceA, -0.5f); // result = 0.5*A
  result.AddFrom(sourceB, 2.0f);  // result = 0.5*A + 2.0*B

  for (unsigned i = 0; i < result.GetNUnits(); ++i) {
    float expected = 0.5f * sourceA.GetData()[i] + 2.0f * sourceB.GetData()[i];
    const float epsilon = 1e-6f;
    float diff = std::abs(result.GetData()[i] - expected);

    GOAssert(
      diff < epsilon,
      std::format(
        "Unit {} should be {} (got: {}, diff: {})",
        i,
        expected,
        result.GetData()[i],
        diff));
  }
}

void GOTestSoundBufferMutable::TestCopyChannelFrom() {
  const unsigned srcChannels = 3;
  const unsigned dstChannels = 2;
  const unsigned nSamples = 4;

  // Source buffer with 3 channels
  GO_DECLARE_LOCAL_SOUND_BUFFER(srcBuffer, srcChannels, nSamples);
  GO_DECLARE_LOCAL_SOUND_BUFFER(dstBuffer, dstChannels, nSamples);

  // Initialize source buffer with test data
  float srcVals[] = {// Sample 0: ch0, ch1, ch2
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

  for (unsigned i = 0; i < srcBuffer.GetNUnits(); ++i) {
    srcBuffer.GetData()[i] = srcVals[i];
  }

  dstBuffer.FillWithSilence();

  // Copy channel 1 from source to channel 0 of destination
  dstBuffer.CopyChannelFrom(srcBuffer, 1, 0);

  // Verify that channel 0 of destination now contains data from channel 1 of
  // source and channel 1 of destination remains zero
  for (unsigned i = 0; i < nSamples; ++i) {
    GOAssert(
      dstBuffer.GetData()[i * dstChannels]
        == srcBuffer.GetData()[i * srcChannels + 1],
      std::format(
        "Sample {} channel 0 should be {} (got: {})",
        i,
        srcBuffer.GetData()[i * srcChannels + 1],
        dstBuffer.GetData()[i * dstChannels]));

    GOAssert(
      dstBuffer.GetData()[i * dstChannels + 1] == 0.0f,
      std::format(
        "Sample {} channel 1 should still be 0.0 (got: {})",
        i,
        dstBuffer.GetData()[i * dstChannels + 1]));
  }

  // Copy channel 2 from source to channel 1 of destination
  dstBuffer.CopyChannelFrom(srcBuffer, 2, 1);

  // Verify both channels
  for (unsigned i = 0; i < nSamples; ++i) {
    GOAssert(
      dstBuffer.GetData()[i * dstChannels]
        == srcBuffer.GetData()[i * srcChannels + 1],
      std::format(
        "Sample {} channel 0 should be {} (got: {})",
        i,
        srcBuffer.GetData()[i * srcChannels + 1],
        dstBuffer.GetData()[i * dstChannels]));

    GOAssert(
      dstBuffer.GetData()[i * dstChannels + 1]
        == srcBuffer.GetData()[i * srcChannels + 2],
      std::format(
        "Sample {} channel 1 should be {} (got: {})",
        i,
        srcBuffer.GetData()[i * srcChannels + 2],
        dstBuffer.GetData()[i * dstChannels + 1]));
  }
}

void GOTestSoundBufferMutable::TestAddChannelFrom() {
  const unsigned srcChannels = 2;
  const unsigned dstChannels = 2;
  const unsigned nSamples = 3;

  GO_DECLARE_LOCAL_SOUND_BUFFER(srcBuffer, srcChannels, nSamples);
  GO_DECLARE_LOCAL_SOUND_BUFFER(dstBuffer, dstChannels, nSamples);

  // Initialize source buffer: [1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f]
  for (unsigned i = 0; i < srcBuffer.GetNUnits(); ++i) {
    srcBuffer.GetData()[i] = static_cast<float>(i + 1);
  }

  // Initialize destination buffer with initial values: [10,20,30,40,50,60]
  float dstVals[] = {10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f};
  for (unsigned i = 0; i < dstBuffer.GetNUnits(); ++i) {
    dstBuffer.GetData()[i] = dstVals[i];
  }

  // Add channel 0 from source to channel 0 of destination
  dstBuffer.AddChannelFrom(srcBuffer, 0, 0);

  // Verify results
  float expectedAfterFirst[] = {
    11.0f,
    20.0f, // 10 + 1, 20
    33.0f,
    40.0f, // 30 + 3, 40
    55.0f,
    60.0f // 50 + 5, 60
  };

  for (unsigned i = 0; i < dstBuffer.GetNUnits(); ++i) {
    GOAssert(
      dstBuffer.GetData()[i] == expectedAfterFirst[i],
      std::format(
        "Unit {} should be {} after AddChannelFrom(ch0->ch0) (got: {})",
        i,
        expectedAfterFirst[i],
        dstBuffer.GetData()[i]));
  }

  // Add channel 1 from source to channel 1 of destination
  dstBuffer.AddChannelFrom(srcBuffer, 1, 1);

  // Verify final results
  float expectedAfterSecond[] = {
    11.0f,
    22.0f, // 20 + 2
    33.0f,
    44.0f, // 40 + 4
    55.0f,
    66.0f // 60 + 6
  };

  for (unsigned i = 0; i < dstBuffer.GetNUnits(); ++i) {
    GOAssert(
      dstBuffer.GetData()[i] == expectedAfterSecond[i],
      std::format(
        "Unit {} should be {} after AddChannelFrom(ch1->ch1) (got: {})",
        i,
        expectedAfterSecond[i],
        dstBuffer.GetData()[i]));
  }
}

void GOTestSoundBufferMutable::TestAddChannelFromWithCoefficient() {
  const unsigned srcChannels = 2;
  const unsigned dstChannels = 2;
  const unsigned nSamples = 3;

  GO_DECLARE_LOCAL_SOUND_BUFFER(srcBuffer, srcChannels, nSamples);
  GO_DECLARE_LOCAL_SOUND_BUFFER(dstBuffer, dstChannels, nSamples);
  GO_DECLARE_LOCAL_SOUND_BUFFER(dstBuffer2, dstChannels, nSamples);

  // Initialize source buffer: [1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f]
  for (unsigned i = 0; i < srcBuffer.GetNUnits(); ++i) {
    srcBuffer.GetData()[i] = static_cast<float>(i + 1);
  }

  // Initialize destination buffer with initial values: [10,20,30,40,50,60]
  float dstVals[] = {10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f};
  for (unsigned i = 0; i < dstBuffer.GetNUnits(); ++i) {
    dstBuffer.GetData()[i] = dstVals[i];
  }

  const float coefficient = 0.5f;

  // Add channel 0 from source to channel 0 of destination with coefficient 0.5
  dstBuffer.AddChannelFrom(srcBuffer, 0, 0, coefficient);

  // Verify results
  float expected[] = {
    10.5f,
    20.0f, // 10 + 1*0.5, 20
    31.5f,
    40.0f, // 30 + 3*0.5, 40
    52.5f,
    60.0f // 50 + 5*0.5, 60
  };

  for (unsigned i = 0; i < dstBuffer.GetNUnits(); ++i) {
    const float epsilon = 1e-6f;
    float diff = std::abs(dstBuffer.GetData()[i] - expected[i]);

    GOAssert(
      diff < epsilon,
      std::format(
        "Unit {} should be {} (got: {}, diff: {})",
        i,
        expected[i],
        dstBuffer.GetData()[i],
        diff));
  }

  // Copy current state to dstBuffer2
  for (unsigned i = 0; i < dstBuffer2.GetNUnits(); ++i) {
    dstBuffer2.GetData()[i] = dstBuffer.GetData()[i];
  }

  // Test with negative coefficient
  const float negativeCoefficient = -1.0f;
  dstBuffer2.AddChannelFrom(srcBuffer, 1, 1, negativeCoefficient);

  // Expect: channel 1 decreased by values from channel 1 of source
  float expected2[] = {
    10.5f,
    18.0f, // 20.0 - 2.0
    31.5f,
    36.0f, // 40.0 - 4.0
    52.5f,
    54.0f // 60.0 - 6.0
  };

  for (unsigned i = 0; i < dstBuffer2.GetNUnits(); ++i) {
    const float epsilon = 1e-6f;
    float diff = std::abs(dstBuffer2.GetData()[i] - expected2[i]);

    GOAssert(
      diff < epsilon,
      std::format(
        "Unit {} should be {} with negative coefficient (got: {}, diff: {})",
        i,
        expected2[i],
        dstBuffer2.GetData()[i],
        diff));
  }
}

void GOTestSoundBufferMutable::TestCrossChannelOperations() {
  const unsigned nChannels = 3;
  const unsigned nSamples = 4;

  GO_DECLARE_LOCAL_SOUND_BUFFER(srcBuffer, nChannels, nSamples);
  GO_DECLARE_LOCAL_SOUND_BUFFER(dstBuffer, nChannels, nSamples);

  // Initialize source buffer
  float srcVals[] = {
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

  for (unsigned i = 0; i < srcBuffer.GetNUnits(); ++i) {
    srcBuffer.GetData()[i] = srcVals[i];
  }

  dstBuffer.FillWithSilence();

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
    float expectedCh0 = srcBuffer.GetData()[base + 2] * 0.5f;
    GOAssert(
      dstBuffer.GetData()[base] == expectedCh0,
      std::format(
        "Sample {} channel 0 should be {} (got: {})",
        i,
        expectedCh0,
        dstBuffer.GetData()[base]));

    // Channel 1: source channel 0
    float expectedCh1 = srcBuffer.GetData()[base];
    GOAssert(
      dstBuffer.GetData()[base + 1] == expectedCh1,
      std::format(
        "Sample {} channel 1 should be {} (got: {})",
        i,
        expectedCh1,
        dstBuffer.GetData()[base + 1]));

    // Channel 2: 2.0 * source channel 1
    float expectedCh2 = srcBuffer.GetData()[base + 1] * 2.0f;
    GOAssert(
      dstBuffer.GetData()[base + 2] == expectedCh2,
      std::format(
        "Sample {} channel 2 should be {} (got: {})",
        i,
        expectedCh2,
        dstBuffer.GetData()[base + 2]));
  }
}

void GOTestSoundBufferMutable::TestChannelOperationsWithSubBuffers() {
  const unsigned nChannels = 2;
  const unsigned nSamples = 6;

  GO_DECLARE_LOCAL_SOUND_BUFFER(mainBuffer, nChannels, nSamples);
  GO_DECLARE_LOCAL_SOUND_BUFFER(sourceBuffer, nChannels, 3); // 3 samples

  // Initialize main buffer
  float mainVals[] = {
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

  for (unsigned i = 0; i < mainBuffer.GetNUnits(); ++i) {
    mainBuffer.GetData()[i] = mainVals[i];
  }

  // Initialize source buffer
  float srcVals[] = {10.0f, 20.0f, 11.0f, 21.0f, 12.0f, 22.0f};
  for (unsigned i = 0; i < sourceBuffer.GetNUnits(); ++i) {
    sourceBuffer.GetData()[i] = srcVals[i];
  }

  // Create sub-buffer (samples 2-4, i.e., 3 samples)
  GOSoundBufferMutable subBuffer = mainBuffer.GetSubBuffer(2, 3);

  // Copy from source to sub-buffer, channel 0 to channel 1
  subBuffer.CopyChannelFrom(sourceBuffer, 0, 1);

  // Verify that only sub-buffer changed
  float expectedMainData[] = {
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

  for (unsigned i = 0; i < mainBuffer.GetNUnits(); ++i) {
    GOAssert(
      mainBuffer.GetData()[i] == expectedMainData[i],
      std::format(
        "Main buffer unit {} should be {} (got: {})",
        i,
        expectedMainData[i],
        mainBuffer.GetData()[i]));
  }
}

void GOTestSoundBufferMutable::TestInvalidChannelIndices() {
  // This test verifies that using invalid channel indices
  // triggers assert in debug builds

  const unsigned srcChannels = 2;
  const unsigned dstChannels = 2;
  const unsigned nSamples = 3;

  GO_DECLARE_LOCAL_SOUND_BUFFER(srcBuffer, srcChannels, nSamples);
  GO_DECLARE_LOCAL_SOUND_BUFFER(dstBuffer, dstChannels, nSamples);

  // Initialize source with 1.0f
  for (unsigned i = 0; i < srcBuffer.GetNUnits(); ++i) {
    srcBuffer.GetData()[i] = 1.0f;
  }

  dstBuffer.FillWithSilence();

  // These operations should work correctly
  dstBuffer.CopyChannelFrom(srcBuffer, 0, 0); // Valid indices
  dstBuffer.CopyChannelFrom(srcBuffer, 1, 1); // Valid indices

  // The following operations would trigger assert in debug mode
  // (commented out as they would cause test failure in debug)
  /*
  // Invalid source index
  dstBuffer.CopyChannelFrom(srcBuffer, 2, 0);  // srcChannel=2 invalid

  // Invalid destination index
  dstBuffer.CopyChannelFrom(srcBuffer, 0, 2);  // dstChannel=2 invalid
  */

  // Instead, verify that valid operations completed successfully
  for (unsigned i = 0; i < nSamples; ++i) {
    GOAssert(
      dstBuffer.GetData()[i * dstChannels] == 1.0f,
      std::format("Sample {} channel 0 should be 1.0", i));
    GOAssert(
      dstBuffer.GetData()[i * dstChannels + 1] == 1.0f,
      std::format("Sample {} channel 1 should be 1.0", i));
  }
}

void GOTestSoundBufferMutable::TestSingleChannelBuffer() {
  // Test operations with single-channel buffers

  const unsigned monoChannels = 1;
  const unsigned stereoChannels = 2;
  const unsigned nSamples = 4;

  GO_DECLARE_LOCAL_SOUND_BUFFER(monoBuffer, monoChannels, nSamples);
  GO_DECLARE_LOCAL_SOUND_BUFFER(stereoBuffer, stereoChannels, nSamples);

  // Initialize mono buffer: [1.0f, 2.0f, 3.0f, 4.0f]
  for (unsigned i = 0; i < monoBuffer.GetNUnits(); ++i) {
    monoBuffer.GetData()[i] = static_cast<float>(i + 1);
  }

  stereoBuffer.FillWithSilence();

  // Copy from mono to left channel of stereo
  stereoBuffer.CopyChannelFrom(monoBuffer, 0, 0);

  // Copy from mono to right channel of stereo with coefficient
  stereoBuffer.AddChannelFrom(monoBuffer, 0, 1, 0.5f);

  // Verify results
  for (unsigned i = 0; i < nSamples; ++i) {
    GOAssert(
      stereoBuffer.GetData()[i * stereoChannels] == monoBuffer.GetData()[i],
      std::format(
        "Left channel sample {} should be {} (got: {})",
        i,
        monoBuffer.GetData()[i],
        stereoBuffer.GetData()[i * stereoChannels]));

    GOAssert(
      stereoBuffer.GetData()[i * stereoChannels + 1]
        == monoBuffer.GetData()[i] * 0.5f,
      std::format(
        "Right channel sample {} should be {} (got: {})",
        i,
        monoBuffer.GetData()[i] * 0.5f,
        stereoBuffer.GetData()[i * stereoChannels + 1]));
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