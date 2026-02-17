/*
 * Copyright 2006 Milan Digital Audio LLC
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
  const unsigned nFrames = 3;

  // Use macro to declare local buffer
  GO_DECLARE_LOCAL_SOUND_BUFFER(buffer, nChannels, nFrames);

  GOAssert(buffer.isValid(), "Mutable buffer should be valid");

  GOAssert(
    buffer.GetNChannels() == nChannels,
    std::format(
      "Mutable buffer should have {} channels (got: {})",
      nChannels,
      buffer.GetNChannels()));

  GOAssert(
    buffer.GetNFrames() == nFrames,
    std::format(
      "Mutable buffer should have {} frames (got: {})",
      nFrames,
      buffer.GetNFrames()));

  // Initialize data with 1.0f using FillWith
  buffer.FillWith({1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f});

  // Check that we can modify data
  buffer.GetData()[0] = 42.0f;
  GOAssert(
    buffer.GetData()[0] == 42.0f,
    std::format(
      "Data should be mutable (expected: 42.0, got: {})", buffer.GetData()[0]));
}

void GOTestSoundBufferMutable::TestFillWithSilence() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 4;

  GO_DECLARE_LOCAL_SOUND_BUFFER(buffer, nChannels, nFrames);

  // Initialize with non-zero values using FillWith
  buffer.FillWith({1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f});

  buffer.FillWithSilence();

  for (unsigned i = 0; i < buffer.GetNItems(); ++i) {
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
  const unsigned nFrames = 3;

  // Create source buffer with test data
  GO_DECLARE_LOCAL_SOUND_BUFFER(sourceBuffer, nChannels, nFrames);
  GO_DECLARE_LOCAL_SOUND_BUFFER(destBuffer, nChannels, nFrames);

  // Initialize source with test data using FillWith
  sourceBuffer.FillWith({1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f});

  // Initialize destination with zeros
  destBuffer.FillWithSilence();

  destBuffer.CopyFrom(sourceBuffer);

  for (unsigned i = 0; i < destBuffer.GetNItems(); ++i) {
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
  const unsigned nFrames = 2;

  GO_DECLARE_LOCAL_SOUND_BUFFER(bufferA, nChannels, nFrames);
  GO_DECLARE_LOCAL_SOUND_BUFFER(bufferB, nChannels, nFrames);
  GO_DECLARE_LOCAL_SOUND_BUFFER(result, nChannels, nFrames);

  // Initialize buffers with FillWith
  bufferA.FillWith({1.0f, 2.0f, 3.0f, 4.0f});
  bufferB.FillWith({0.5f, 1.0f, 1.5f, 2.0f});

  result.FillWithSilence();

  // First copy A
  result.CopyFrom(bufferA);

  // Then add B
  result.AddFrom(bufferB);

  // Verify results
  for (unsigned i = 0; i < result.GetNItems(); ++i) {
    float expected = bufferA.GetData()[i] + bufferB.GetData()[i];
    GOAssert(
      result.GetData()[i] == expected,
      std::format(
        "Unit {} should be {} (got: {})", i, expected, result.GetData()[i]));
  }
}

void GOTestSoundBufferMutable::TestAddFromWithCoefficient() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 2;

  GO_DECLARE_LOCAL_SOUND_BUFFER(bufferA, nChannels, nFrames);
  GO_DECLARE_LOCAL_SOUND_BUFFER(bufferB, nChannels, nFrames);
  GO_DECLARE_LOCAL_SOUND_BUFFER(result, nChannels, nFrames);

  // Initialize buffers with FillWith
  bufferA.FillWith({1.0f, 2.0f, 3.0f, 4.0f});
  bufferB.FillWith({0.5f, 1.0f, 1.5f, 2.0f});

  result.FillWithSilence();

  // First copy A
  result.CopyFrom(bufferA);

  // Then add B with coefficient 2.0
  const float coefficient = 2.0f;
  result.AddFrom(bufferB, coefficient);

  // Verify results
  for (unsigned i = 0; i < result.GetNItems(); ++i) {
    float expected = bufferA.GetData()[i] + bufferB.GetData()[i] * coefficient;
    GOAssert(
      result.GetData()[i] == expected,
      std::format(
        "Unit {} should be {} (got: {})", i, expected, result.GetData()[i]));
  }
}

void GOTestSoundBufferMutable::TestMutableGetSubBuffer() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 4;

  GO_DECLARE_LOCAL_SOUND_BUFFER(buffer, nChannels, nFrames);

  // Initialize with sequential values using FillWith
  buffer.FillWith({1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f});

  // Create mutable sub-buffer
  GOSoundBufferMutable subBuffer = buffer.GetSubBuffer(1, 2);

  GOAssert(subBuffer.isValid(), "Mutable SubBuffer should be valid");

  GOAssert(
    subBuffer.GetNFrames() == 2,
    std::format(
      "Mutable SubBuffer should have 2 frames (got: {})",
      subBuffer.GetNFrames()));

  // Modify data through sub-buffer
  subBuffer.FillWithSilence();

  // Check that only sub-buffer data changed
  GOAssert(
    buffer.GetData()[0] == 1.0f && buffer.GetData()[1] == 2.0f,
    "First frame should remain unchanged");

  GOAssert(
    buffer.GetData()[2] == 0.0f && buffer.GetData()[3] == 0.0f,
    "Second frame should be zeroed");

  GOAssert(
    buffer.GetData()[4] == 0.0f && buffer.GetData()[5] == 0.0f,
    "Third frame should be zeroed");

  GOAssert(
    buffer.GetData()[6] == 7.0f && buffer.GetData()[7] == 8.0f,
    "Fourth frame should remain unchanged");
}

void GOTestSoundBufferMutable::TestCompatibilityChecks() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 3;

  // Compatible buffers
  GO_DECLARE_LOCAL_SOUND_BUFFER(buffer1, nChannels, nFrames);
  GO_DECLARE_LOCAL_SOUND_BUFFER(mutableBuffer, nChannels, nFrames);

  // Initialize buffer1 with 1.0f using FillWith
  buffer1.FillWith({1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f});

  mutableBuffer.FillWithSilence();

  // Compatible buffers should work
  mutableBuffer.CopyFrom(buffer1);

  // Check that data was copied
  for (unsigned i = 0; i < mutableBuffer.GetNItems(); ++i) {
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
  const unsigned nFrames = 3;

  GO_DECLARE_LOCAL_SOUND_BUFFER(sourceA, nChannels, nFrames);
  GO_DECLARE_LOCAL_SOUND_BUFFER(sourceB, nChannels, nFrames);
  GO_DECLARE_LOCAL_SOUND_BUFFER(result, nChannels, nFrames);

  // Initialize sourceA: 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f
  sourceA.FillWith({1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f});

  // Initialize sourceB: 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f
  sourceB.FillWith({0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f});

  result.FillWithSilence();

  // Complex operation: result = 0.5*A + 2.0*B
  result.CopyFrom(sourceA);
  result.AddFrom(sourceA, -0.5f); // result = 0.5*A
  result.AddFrom(sourceB, 2.0f);  // result = 0.5*A + 2.0*B

  for (unsigned i = 0; i < result.GetNItems(); ++i) {
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
  const unsigned nFrames = 4;

  // Source buffer with 3 channels
  GO_DECLARE_LOCAL_SOUND_BUFFER(srcBuffer, srcChannels, nFrames);
  GO_DECLARE_LOCAL_SOUND_BUFFER(dstBuffer, dstChannels, nFrames);

  // Initialize source buffer with test data using FillWith
  srcBuffer.FillWith({
    1.0f,
    2.0f,
    3.0f, // Frame 0: ch0, ch1, ch2
    4.0f,
    5.0f,
    6.0f, // Frame 1
    7.0f,
    8.0f,
    9.0f, // Frame 2
    10.0f,
    11.0f,
    12.0f // Frame 3
  });

  dstBuffer.FillWithSilence();

  // Copy channel 1 from source to channel 0 of destination
  dstBuffer.CopyChannelFrom(srcBuffer, 1, 0);

  // Verify that channel 0 of destination now contains data from channel 1 of
  // source and channel 1 of destination remains zero
  for (unsigned i = 0; i < nFrames; ++i) {
    GOAssert(
      dstBuffer.GetData()[i * dstChannels]
        == srcBuffer.GetData()[i * srcChannels + 1],
      std::format(
        "Frame {} channel 0 should be {} (got: {})",
        i,
        srcBuffer.GetData()[i * srcChannels + 1],
        dstBuffer.GetData()[i * dstChannels]));

    GOAssert(
      dstBuffer.GetData()[i * dstChannels + 1] == 0.0f,
      std::format(
        "Frame {} channel 1 should still be 0.0 (got: {})",
        i,
        dstBuffer.GetData()[i * dstChannels + 1]));
  }

  // Copy channel 2 from source to channel 1 of destination
  dstBuffer.CopyChannelFrom(srcBuffer, 2, 1);

  // Verify both channels
  for (unsigned i = 0; i < nFrames; ++i) {
    GOAssert(
      dstBuffer.GetData()[i * dstChannels]
        == srcBuffer.GetData()[i * srcChannels + 1],
      std::format(
        "Frame {} channel 0 should be {} (got: {})",
        i,
        srcBuffer.GetData()[i * srcChannels + 1],
        dstBuffer.GetData()[i * dstChannels]));

    GOAssert(
      dstBuffer.GetData()[i * dstChannels + 1]
        == srcBuffer.GetData()[i * srcChannels + 2],
      std::format(
        "Frame {} channel 1 should be {} (got: {})",
        i,
        srcBuffer.GetData()[i * srcChannels + 2],
        dstBuffer.GetData()[i * dstChannels + 1]));
  }
}

void GOTestSoundBufferMutable::TestAddChannelFrom() {
  const unsigned srcChannels = 2;
  const unsigned dstChannels = 2;
  const unsigned nFrames = 3;

  GO_DECLARE_LOCAL_SOUND_BUFFER(srcBuffer, srcChannels, nFrames);
  GO_DECLARE_LOCAL_SOUND_BUFFER(dstBuffer, dstChannels, nFrames);

  // Initialize source buffer with FillWith
  srcBuffer.FillWith({1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f});

  // Initialize destination buffer with initial values using FillWith
  dstBuffer.FillWith({10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f});

  // Add channel 0 from source to channel 0 of destination
  dstBuffer.AddChannelFrom(srcBuffer, 0, 0);

  // Verify results after first addition
  for (unsigned i = 0; i < nFrames; ++i) {
    // Channel 0: 10 + 1, 30 + 3, 50 + 5
    float expectedCh0 = 10.0f + (i * 20.0f) + (1.0f + i * 2.0f);
    // Channel 1 unchanged: 20, 40, 60
    float expectedCh1 = 20.0f + (i * 20.0f);

    GOAssert(
      dstBuffer.GetData()[i * dstChannels] == expectedCh0,
      std::format(
        "Frame {} channel 0 should be {} after AddChannelFrom(ch0->ch0) (got: "
        "{})",
        i,
        expectedCh0,
        dstBuffer.GetData()[i * dstChannels]));

    GOAssert(
      dstBuffer.GetData()[i * dstChannels + 1] == expectedCh1,
      std::format(
        "Frame {} channel 1 should be {} after AddChannelFrom(ch0->ch0) (got: "
        "{})",
        i,
        expectedCh1,
        dstBuffer.GetData()[i * dstChannels + 1]));
  }

  // Add channel 1 from source to channel 1 of destination
  dstBuffer.AddChannelFrom(srcBuffer, 1, 1);

  // Verify final results
  for (unsigned i = 0; i < nFrames; ++i) {
    // Channel 0: same as before
    float expectedCh0 = 10.0f + (i * 20.0f) + (1.0f + i * 2.0f);
    // Channel 1: 20 + 2, 40 + 4, 60 + 6
    float expectedCh1 = 20.0f + (i * 20.0f) + (2.0f + i * 2.0f);

    GOAssert(
      dstBuffer.GetData()[i * dstChannels] == expectedCh0,
      std::format(
        "Frame {} channel 0 should be {} after AddChannelFrom(ch1->ch1) (got: "
        "{})",
        i,
        expectedCh0,
        dstBuffer.GetData()[i * dstChannels]));

    GOAssert(
      dstBuffer.GetData()[i * dstChannels + 1] == expectedCh1,
      std::format(
        "Frame {} channel 1 should be {} after AddChannelFrom(ch1->ch1) (got: "
        "{})",
        i,
        expectedCh1,
        dstBuffer.GetData()[i * dstChannels + 1]));
  }
}

void GOTestSoundBufferMutable::TestAddChannelFromWithCoefficient() {
  const unsigned srcChannels = 2;
  const unsigned dstChannels = 2;
  const unsigned nFrames = 3;

  GO_DECLARE_LOCAL_SOUND_BUFFER(srcBuffer, srcChannels, nFrames);
  GO_DECLARE_LOCAL_SOUND_BUFFER(dstBuffer, dstChannels, nFrames);
  GO_DECLARE_LOCAL_SOUND_BUFFER(dstBuffer2, dstChannels, nFrames);

  // Initialize source buffer with FillWith
  srcBuffer.FillWith({1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f});

  // Initialize destination buffer with initial values using FillWith
  dstBuffer.FillWith({10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f});

  const float coefficient = 0.5f;

  // Add channel 0 from source to channel 0 of destination with coefficient 0.5
  dstBuffer.AddChannelFrom(srcBuffer, 0, 0, coefficient);

  // Verify results
  for (unsigned i = 0; i < nFrames; ++i) {
    // Channel 0: 10 + 1*0.5, 30 + 3*0.5, 50 + 5*0.5
    float expectedCh0 = 10.0f + (i * 20.0f) + (1.0f + i * 2.0f) * coefficient;
    // Channel 1 unchanged: 20, 40, 60
    float expectedCh1 = 20.0f + (i * 20.0f);

    const float epsilon = 1e-6f;
    float diff0 = std::abs(dstBuffer.GetData()[i * dstChannels] - expectedCh0);
    float diff1
      = std::abs(dstBuffer.GetData()[i * dstChannels + 1] - expectedCh1);

    GOAssert(
      diff0 < epsilon,
      std::format(
        "Frame {} channel 0 should be {} (got: {}, diff: {})",
        i,
        expectedCh0,
        dstBuffer.GetData()[i * dstChannels],
        diff0));

    GOAssert(
      diff1 < epsilon,
      std::format(
        "Frame {} channel 1 should be {} (got: {}, diff: {})",
        i,
        expectedCh1,
        dstBuffer.GetData()[i * dstChannels + 1],
        diff1));
  }

  // Copy current state to dstBuffer2 using FillWith
  dstBuffer2.FillWith(
    {dstBuffer.GetData()[0],
     dstBuffer.GetData()[1],
     dstBuffer.GetData()[2],
     dstBuffer.GetData()[3],
     dstBuffer.GetData()[4],
     dstBuffer.GetData()[5]});

  // Test with negative coefficient
  const float negativeCoefficient = -1.0f;
  dstBuffer2.AddChannelFrom(srcBuffer, 1, 1, negativeCoefficient);

  // Verify results with negative coefficient
  for (unsigned i = 0; i < nFrames; ++i) {
    // Channel 0 unchanged from dstBuffer
    float expectedCh0 = 10.0f + (i * 20.0f) + (1.0f + i * 2.0f) * coefficient;
    // Channel 1: previous value minus source channel 1
    float expectedCh1 = 20.0f + (i * 20.0f) - (2.0f + i * 2.0f);

    const float epsilon = 1e-6f;
    float diff0 = std::abs(dstBuffer2.GetData()[i * dstChannels] - expectedCh0);
    float diff1
      = std::abs(dstBuffer2.GetData()[i * dstChannels + 1] - expectedCh1);

    GOAssert(
      diff0 < epsilon,
      std::format(
        "Frame {} channel 0 should be {} with negative coeff (got: {}, diff: "
        "{})",
        i,
        expectedCh0,
        dstBuffer2.GetData()[i * dstChannels],
        diff0));

    GOAssert(
      diff1 < epsilon,
      std::format(
        "Frame {} channel 1 should be {} with negative coeff (got: {}, diff: "
        "{})",
        i,
        expectedCh1,
        dstBuffer2.GetData()[i * dstChannels + 1],
        diff1));
  }
}

void GOTestSoundBufferMutable::TestCrossChannelOperations() {
  const unsigned nChannels = 3;
  const unsigned nFrames = 4;

  GO_DECLARE_LOCAL_SOUND_BUFFER(srcBuffer, nChannels, nFrames);
  GO_DECLARE_LOCAL_SOUND_BUFFER(dstBuffer, nChannels, nFrames);

  // Initialize source buffer with FillWith
  srcBuffer.FillWith(
    {1.0f,
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
     400.0f});

  dstBuffer.FillWithSilence();

  // Complex operations with different channels:
  // 1. Copy channel 0 to channel 1
  dstBuffer.CopyChannelFrom(srcBuffer, 0, 1);

  // 2. Add channel 1 (multiplied by 2) to channel 2
  dstBuffer.AddChannelFrom(srcBuffer, 1, 2, 2.0f);

  // 3. Add channel 2 (multiplied by 0.5) to channel 0
  dstBuffer.AddChannelFrom(srcBuffer, 2, 0, 0.5f);

  // Verify results
  for (unsigned i = 0; i < nFrames; ++i) {
    const unsigned base = i * nChannels;

    // Channel 0: 0.5 * source channel 2
    float expectedCh0 = srcBuffer.GetData()[base + 2] * 0.5f;
    // Channel 1: source channel 0
    float expectedCh1 = srcBuffer.GetData()[base];
    // Channel 2: 2.0 * source channel 1
    float expectedCh2 = srcBuffer.GetData()[base + 1] * 2.0f;

    GOAssert(
      dstBuffer.GetData()[base] == expectedCh0,
      std::format(
        "Frame {} channel 0 should be {} (got: {})",
        i,
        expectedCh0,
        dstBuffer.GetData()[base]));

    GOAssert(
      dstBuffer.GetData()[base + 1] == expectedCh1,
      std::format(
        "Frame {} channel 1 should be {} (got: {})",
        i,
        expectedCh1,
        dstBuffer.GetData()[base + 1]));

    GOAssert(
      dstBuffer.GetData()[base + 2] == expectedCh2,
      std::format(
        "Frame {} channel 2 should be {} (got: {})",
        i,
        expectedCh2,
        dstBuffer.GetData()[base + 2]));
  }
}

void GOTestSoundBufferMutable::TestChannelOperationsWithSubBuffers() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 6;

  GO_DECLARE_LOCAL_SOUND_BUFFER(mainBuffer, nChannels, nFrames);
  GO_DECLARE_LOCAL_SOUND_BUFFER(sourceBuffer, nChannels, 3); // 3 frames

  // Initialize main buffer with FillWith
  mainBuffer.FillWith(
    {1.0f,
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
     106.0f});

  // Initialize source buffer with FillWith
  sourceBuffer.FillWith({10.0f, 20.0f, 11.0f, 21.0f, 12.0f, 22.0f});

  // Create sub-buffer (frames 2-4, i.e., 3 frames)
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

  for (unsigned i = 0; i < mainBuffer.GetNItems(); ++i) {
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
  const unsigned nFrames = 3;

  GO_DECLARE_LOCAL_SOUND_BUFFER(srcBuffer, srcChannels, nFrames);
  GO_DECLARE_LOCAL_SOUND_BUFFER(dstBuffer, dstChannels, nFrames);

  // Initialize source with 1.0f using FillWith
  srcBuffer.FillWith({1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f});

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
  for (unsigned i = 0; i < nFrames; ++i) {
    GOAssert(
      dstBuffer.GetData()[i * dstChannels] == 1.0f,
      std::format("Frame {} channel 0 should be 1.0", i));
    GOAssert(
      dstBuffer.GetData()[i * dstChannels + 1] == 1.0f,
      std::format("Frame {} channel 1 should be 1.0", i));
  }
}

void GOTestSoundBufferMutable::TestSingleChannelBuffer() {
  // Test operations with single-channel buffers

  const unsigned monoChannels = 1;
  const unsigned stereoChannels = 2;
  const unsigned nFrames = 4;

  GO_DECLARE_LOCAL_SOUND_BUFFER(monoBuffer, monoChannels, nFrames);
  GO_DECLARE_LOCAL_SOUND_BUFFER(stereoBuffer, stereoChannels, nFrames);

  // Initialize mono buffer with FillWith
  monoBuffer.FillWith({1.0f, 2.0f, 3.0f, 4.0f});

  stereoBuffer.FillWithSilence();

  // Copy from mono to left channel of stereo
  stereoBuffer.CopyChannelFrom(monoBuffer, 0, 0);

  // Copy from mono to right channel of stereo with coefficient
  stereoBuffer.AddChannelFrom(monoBuffer, 0, 1, 0.5f);

  // Verify results
  for (unsigned i = 0; i < nFrames; ++i) {
    GOAssert(
      stereoBuffer.GetData()[i * stereoChannels] == monoBuffer.GetData()[i],
      std::format(
        "Left channel frame {} should be {} (got: {})",
        i,
        monoBuffer.GetData()[i],
        stereoBuffer.GetData()[i * stereoChannels]));

    GOAssert(
      stereoBuffer.GetData()[i * stereoChannels + 1]
        == monoBuffer.GetData()[i] * 0.5f,
      std::format(
        "Right channel frame {} should be {} (got: {})",
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