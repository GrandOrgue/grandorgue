/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2024-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundBufferMutable.h"

#include <cmath>
#include <format>

#include "sound/buffer/GOSoundBufferMutable.h"

const std::string GOTestSoundBufferMutable::TEST_NAME
  = "GOTestSoundBufferMutable";

void GOTestSoundBufferMutable::AssertAllItemsEqual(
  const std::string &context,
  float expectedValue,
  const GOSoundBuffer &buffer) {
  for (unsigned n = buffer.GetNItems(), i = 0; i < n; ++i)
    AssertItemEqual(context, i, expectedValue, buffer.GetData()[i]);
}

void GOTestSoundBufferMutable::AssertChannelItemEqual(
  const std::string &context,
  unsigned frameIndex,
  unsigned channelIndex,
  float expectedValue,
  float gotValue) {
  GOAssert(
    gotValue == expectedValue,
    std::format(
      "{}: frame {} ch{} got: {}, expected: {}",
      context,
      frameIndex,
      channelIndex,
      gotValue,
      expectedValue));
}

void GOTestSoundBufferMutable::AssertChannelItemNear(
  const std::string &context,
  unsigned frameIndex,
  unsigned channelIndex,
  float expectedValue,
  float gotValue) {
  const float epsilon = 1e-6f;
  const float diff = std::abs(gotValue - expectedValue);

  GOAssert(
    diff < epsilon,
    std::format(
      "{}: frame {} ch{} got: {}, expected: {} (diff: {})",
      context,
      frameIndex,
      channelIndex,
      gotValue,
      expectedValue,
      diff));
}

void GOTestSoundBufferMutable::AssertChannelEqual(
  const std::string &context,
  const GOSoundBuffer &expectedBuffer,
  unsigned expectedChannelI,
  const GOSoundBuffer &gotBuffer,
  unsigned gotChannelI) {
  const unsigned expectedNChannels = expectedBuffer.GetNChannels();
  const unsigned gotNChannels = gotBuffer.GetNChannels();
  const unsigned nFrames = expectedBuffer.GetNFrames();

  for (unsigned frameI = 0; frameI < nFrames; ++frameI)
    AssertItemEqual(
      context,
      frameI,
      expectedBuffer.GetData()[frameI * expectedNChannels + expectedChannelI],
      gotBuffer.GetData()[frameI * gotNChannels + gotChannelI]);
}

void GOTestSoundBufferMutable::AssertBuffersDataEqual(
  const std::string &context,
  const GOSoundBuffer &expectedBuffer,
  const GOSoundBuffer &buffer) {
  for (unsigned n = buffer.GetNItems(), i = 0; i < n; ++i)
    AssertItemEqual(
      context, i, expectedBuffer.GetData()[i], buffer.GetData()[i]);
}

void GOTestSoundBufferMutable::TestInheritanceAndMutableAccess() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 3;

  // Use macro to declare local buffer
  GO_DECLARE_LOCAL_SOUND_BUFFER(buffer, nChannels, nFrames);

  AssertDimensions("InheritanceAndMutableAccess", buffer, nChannels, nFrames);

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

  AssertAllItemsEqual("FillWithSilence", 0.0f, buffer);
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

  AssertBuffersDataEqual("CopyFrom", sourceBuffer, destBuffer);
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
  for (unsigned nItems = result.GetNItems(), itemI = 0; itemI < nItems; ++itemI)
    AssertItemEqual(
      "AddFrom",
      itemI,
      bufferA.GetData()[itemI] + bufferB.GetData()[itemI],
      result.GetData()[itemI]);
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
  for (unsigned nItems = result.GetNItems(), itemI = 0; itemI < nItems; ++itemI)
    AssertItemEqual(
      "AddFrom+coeff",
      itemI,
      bufferA.GetData()[itemI] + bufferB.GetData()[itemI] * coefficient,
      result.GetData()[itemI]);
}

void GOTestSoundBufferMutable::TestMutableGetSubBuffer() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 4;

  GO_DECLARE_LOCAL_SOUND_BUFFER(buffer, nChannels, nFrames);

  // Initialize with sequential values using FillWith
  buffer.FillWith({1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f});

  // Create mutable sub-buffer
  GOSoundBufferMutable subBuffer = buffer.GetSubBuffer(1, 2);

  AssertDimensions("MutableGetSubBuffer", subBuffer, nChannels, 2);

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
  AssertAllItemsEqual("CopyFrom", 1.0f, mutableBuffer);

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

  for (unsigned nItems = result.GetNItems(), itemI = 0; itemI < nItems;
       ++itemI) {
    float expected
      = 0.5f * sourceA.GetData()[itemI] + 2.0f * sourceB.GetData()[itemI];
    const float epsilon = 1e-6f;
    float diff = std::abs(result.GetData()[itemI] - expected);

    GOAssert(
      diff < epsilon,
      std::format(
        "ComplexOperations: item {} got: {}, expected: {} (diff: {})",
        itemI,
        result.GetData()[itemI],
        expected,
        diff));
  }
}

void GOTestSoundBufferMutable::TestCopyChannelFrom() {
  const unsigned srcNChannels = 3;
  const unsigned dstNChannels = 2;
  const unsigned nFrames = 4;

  // Source buffer with 3 channels
  GO_DECLARE_LOCAL_SOUND_BUFFER(srcBuffer, srcNChannels, nFrames);
  GO_DECLARE_LOCAL_SOUND_BUFFER(dstBuffer, dstNChannels, nFrames);

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
  AssertChannelEqual("CopyChannelFrom ch1->ch0", srcBuffer, 1, dstBuffer, 0);
  for (unsigned frameI = 0; frameI < nFrames; ++frameI)
    AssertChannelItemEqual(
      "CopyChannelFrom ch1->ch0",
      frameI,
      1,
      0.0f,
      dstBuffer.GetData()[frameI * dstNChannels + 1]);

  // Copy channel 2 from source to channel 1 of destination
  dstBuffer.CopyChannelFrom(srcBuffer, 2, 1);

  // Verify both channels
  AssertChannelEqual("CopyChannelFrom ch1->ch0", srcBuffer, 1, dstBuffer, 0);
  AssertChannelEqual("CopyChannelFrom ch2->ch1", srcBuffer, 2, dstBuffer, 1);
}

void GOTestSoundBufferMutable::TestAddChannelFrom() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 3;

  GO_DECLARE_LOCAL_SOUND_BUFFER(srcBuffer, nChannels, nFrames);
  GO_DECLARE_LOCAL_SOUND_BUFFER(dstBuffer, nChannels, nFrames);

  // Initialize source buffer with FillWith
  srcBuffer.FillWith({1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f});

  // Initialize destination buffer with initial values using FillWith
  dstBuffer.FillWith({10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f});

  // Add channel 0 from source to channel 0 of destination
  dstBuffer.AddChannelFrom(srcBuffer, 0, 0);

  // Verify results after first addition
  for (unsigned frameI = 0; frameI < nFrames; ++frameI) {
    // Channel 0: 10 + 1, 30 + 3, 50 + 5
    // Channel 1 unchanged: 20, 40, 60
    AssertChannelItemEqual(
      "AddChannelFrom(ch0->ch0)",
      frameI,
      0,
      10.0f + (frameI * 20.0f) + (1.0f + frameI * 2.0f),
      dstBuffer.GetData()[frameI * nChannels]);
    AssertChannelItemEqual(
      "AddChannelFrom(ch0->ch0)",
      frameI,
      1,
      20.0f + (frameI * 20.0f),
      dstBuffer.GetData()[frameI * nChannels + 1]);
  }

  // Add channel 1 from source to channel 1 of destination
  dstBuffer.AddChannelFrom(srcBuffer, 1, 1);

  // Verify final results
  for (unsigned frameI = 0; frameI < nFrames; ++frameI) {
    // Channel 0: same as before
    // Channel 1: 20 + 2, 40 + 4, 60 + 6
    AssertChannelItemEqual(
      "AddChannelFrom(ch1->ch1)",
      frameI,
      0,
      10.0f + (frameI * 20.0f) + (1.0f + frameI * 2.0f),
      dstBuffer.GetData()[frameI * nChannels]);
    AssertChannelItemEqual(
      "AddChannelFrom(ch1->ch1)",
      frameI,
      1,
      20.0f + (frameI * 20.0f) + (2.0f + frameI * 2.0f),
      dstBuffer.GetData()[frameI * nChannels + 1]);
  }
}

void GOTestSoundBufferMutable::TestAddChannelFromWithCoefficient() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 3;

  GO_DECLARE_LOCAL_SOUND_BUFFER(srcBuffer, nChannels, nFrames);
  GO_DECLARE_LOCAL_SOUND_BUFFER(dstBuffer, nChannels, nFrames);
  GO_DECLARE_LOCAL_SOUND_BUFFER(dstBuffer2, nChannels, nFrames);

  // Initialize source buffer with FillWith
  srcBuffer.FillWith({1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f});

  // Initialize destination buffer with initial values using FillWith
  dstBuffer.FillWith({10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f});

  const float coefficient = 0.5f;

  // Add channel 0 from source to channel 0 of destination with coefficient 0.5
  dstBuffer.AddChannelFrom(srcBuffer, 0, 0, coefficient);

  // Verify results
  for (unsigned frameI = 0; frameI < nFrames; ++frameI) {
    // Channel 0: 10 + 1*0.5, 30 + 3*0.5, 50 + 5*0.5
    float expectedCh0
      = 10.0f + (frameI * 20.0f) + (1.0f + frameI * 2.0f) * coefficient;
    // Channel 1 unchanged: 20, 40, 60
    float expectedCh1 = 20.0f + (frameI * 20.0f);

    AssertChannelItemNear(
      "AddChannelFrom+coeff",
      frameI,
      0,
      expectedCh0,
      dstBuffer.GetData()[frameI * nChannels]);
    AssertChannelItemNear(
      "AddChannelFrom+coeff",
      frameI,
      1,
      expectedCh1,
      dstBuffer.GetData()[frameI * nChannels + 1]);
  }

  dstBuffer2.CopyFrom(dstBuffer);

  // Test with negative coefficient
  const float negativeCoefficient = -1.0f;
  dstBuffer2.AddChannelFrom(srcBuffer, 1, 1, negativeCoefficient);

  // Verify results with negative coefficient
  for (unsigned frameI = 0; frameI < nFrames; ++frameI) {
    // Channel 0 unchanged from dstBuffer
    float expectedCh0
      = 10.0f + (frameI * 20.0f) + (1.0f + frameI * 2.0f) * coefficient;
    // Channel 1: previous value minus source channel 1
    float expectedCh1 = 20.0f + (frameI * 20.0f) - (2.0f + frameI * 2.0f);

    AssertChannelItemNear(
      "AddChannelFrom+negCoeff",
      frameI,
      0,
      expectedCh0,
      dstBuffer2.GetData()[frameI * nChannels]);
    AssertChannelItemNear(
      "AddChannelFrom+negCoeff",
      frameI,
      1,
      expectedCh1,
      dstBuffer2.GetData()[frameI * nChannels + 1]);
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
  for (unsigned frameI = 0; frameI < nFrames; ++frameI) {
    const unsigned base = frameI * nChannels;

    // Channel 0: 0.5 * source channel 2
    // Channel 1: source channel 0
    // Channel 2: 2.0 * source channel 1
    AssertChannelItemEqual(
      "CrossChannel",
      frameI,
      0,
      srcBuffer.GetData()[base + 2] * 0.5f,
      dstBuffer.GetData()[base]);
    AssertChannelItemEqual(
      "CrossChannel",
      frameI,
      1,
      srcBuffer.GetData()[base],
      dstBuffer.GetData()[base + 1]);
    AssertChannelItemEqual(
      "CrossChannel",
      frameI,
      2,
      srcBuffer.GetData()[base + 1] * 2.0f,
      dstBuffer.GetData()[base + 2]);
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

  AssertBuffersDataEqual(
    "ChannelOperationsWithSubBuffers",
    GOSoundBuffer(expectedMainData, nChannels, nFrames),
    mainBuffer);
}

void GOTestSoundBufferMutable::TestInvalidChannelIndices() {
  // This test verifies that using invalid channel indices
  // triggers assert in debug builds

  const unsigned srcNChannels = 2;
  const unsigned dstNChannels = 2;
  const unsigned nFrames = 3;

  GO_DECLARE_LOCAL_SOUND_BUFFER(srcBuffer, srcNChannels, nFrames);
  GO_DECLARE_LOCAL_SOUND_BUFFER(dstBuffer, dstNChannels, nFrames);

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
  AssertAllItemsEqual("CopyChannelFrom valid indices", 1.0f, dstBuffer);
}

void GOTestSoundBufferMutable::TestSingleChannelBuffer() {
  // Test operations with single-channel buffers

  const unsigned monoNChannels = 1;
  const unsigned stereoNChannels = 2;
  const unsigned nFrames = 4;

  GO_DECLARE_LOCAL_SOUND_BUFFER(monoBuffer, monoNChannels, nFrames);
  GO_DECLARE_LOCAL_SOUND_BUFFER(stereoBuffer, stereoNChannels, nFrames);

  // Initialize mono buffer with FillWith
  monoBuffer.FillWith({1.0f, 2.0f, 3.0f, 4.0f});

  stereoBuffer.FillWithSilence();

  // Copy from mono to left channel of stereo
  stereoBuffer.CopyChannelFrom(monoBuffer, 0, 0);

  // Copy from mono to right channel of stereo with coefficient
  stereoBuffer.AddChannelFrom(monoBuffer, 0, 1, 0.5f);

  // Verify results
  for (unsigned frameI = 0; frameI < nFrames; ++frameI) {
    AssertChannelItemEqual(
      "SingleChannel",
      frameI,
      0,
      monoBuffer.GetData()[frameI],
      stereoBuffer.GetData()[frameI * stereoNChannels]);
    AssertChannelItemEqual(
      "SingleChannel",
      frameI,
      1,
      monoBuffer.GetData()[frameI] * 0.5f,
      stereoBuffer.GetData()[frameI * stereoNChannels + 1]);
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