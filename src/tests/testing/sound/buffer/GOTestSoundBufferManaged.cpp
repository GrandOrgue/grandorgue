/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2024-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundBufferManaged.h"

#include <cmath>
#include <format>
#include <vector>

#include "sound/buffer/GOSoundBufferManaged.h"

const std::string GOTestSoundBufferManaged::TEST_NAME
  = "GOTestSoundBufferManaged";

void GOTestSoundBufferManaged::TestDefaultConstructor() {
  TestManagedDefaultConstructor<GOSoundBufferManaged>(
    "Managed DefaultConstructor");
}

void GOTestSoundBufferManaged::TestConstructorWithDimensions() {
  TestManagedConstructorWithDimensions<GOSoundBufferManaged>(
    "Managed ConstructorWithDimensions");
}

void GOTestSoundBufferManaged::TestCopyConstructorFromBuffer() {
  TestManagedCopyConstructorFromView<GOSoundBufferManaged, GOSoundBuffer>(
    "Managed CopyConstructorFromBuffer");
}

void GOTestSoundBufferManaged::TestCopyConstructorFromManaged() {
  TestManagedCopyConstructorFromManaged<GOSoundBufferManaged>(
    "Managed CopyConstructorFromManaged");
}

void GOTestSoundBufferManaged::TestMoveConstructor() {
  TestManagedMoveConstructor<GOSoundBufferManaged>("Managed MoveConstructor");
}

void GOTestSoundBufferManaged::TestCopyAssignmentFromBuffer() {
  TestManagedCopyAssignmentFromView<GOSoundBufferManaged, GOSoundBuffer>(
    "Managed CopyAssignmentFromBuffer");
}

void GOTestSoundBufferManaged::TestCopyAssignmentFromManaged() {
  TestManagedCopyAssignmentFromManaged<GOSoundBufferManaged>(
    "Managed CopyAssignmentFromManaged");
}

void GOTestSoundBufferManaged::TestMoveAssignment() {
  TestManagedMoveAssignment<GOSoundBufferManaged>("Managed MoveAssignment");
}

void GOTestSoundBufferManaged::TestResize() {
  TestManagedResize<GOSoundBufferManaged>("Managed Resize");
}

void GOTestSoundBufferManaged::TestResizeNoReallocation() {
  TestManagedResizeNoReallocation<GOSoundBufferManaged>(
    "Managed ResizeNoReallocation");
}

void GOTestSoundBufferManaged::TestSwap() {
  TestManagedSwap<GOSoundBufferManaged>("Managed Swap");
}

void GOTestSoundBufferManaged::TestComplexOperations() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 3;
  const unsigned nItems = nChannels * nFrames;

  // Create source buffer with external memory
  std::vector<GOSoundBuffer::Item> sourceData(nItems);

  fillWithSequential(sourceData.data(), nItems, 1.0f);

  GOSoundBuffer sourceBuffer(sourceData.data(), nChannels, nFrames);

  // Create managed buffer and copy data
  GOSoundBufferManaged buffer(sourceBuffer);

  // Modify managed buffer
  buffer.AddFrom(sourceBuffer, 0.5f); // buffer = 1.5 * source

  // Create another managed buffer
  GOSoundBufferManaged buffer2(2, 3);
  buffer2.FillWithSilence();
  buffer2.AddFrom(buffer, 2.0f); // buffer2 = 3.0 * source

  // Verify calculations
  for (unsigned itemI = 0; itemI < nItems; ++itemI) {
    float expected = static_cast<float>(itemI + 1) * 3.0f;
    const float epsilon = 1e-6f;
    float diff = std::abs(buffer2.GetData()[itemI] - expected);

    GOAssert(
      diff < epsilon,
      std::format(
        "ComplexOperations: item {} got: {}, expected: {} (diff: {})",
        itemI,
        buffer2.GetData()[itemI],
        expected,
        diff));
  }
}

void GOTestSoundBufferManaged::TestSubBufferCompatibility() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 4;

  GOSoundBufferManaged buffer(nChannels, nFrames);

  // Get a sub-buffer (mutable)
  GOSoundBufferMutable subBuffer = buffer.GetSubBuffer(1, 2);

  GOAssert(subBuffer.isValid(), "Sub-buffer should be valid");
  GOAssert(
    subBuffer.GetNFrames() == 2,
    std::format(
      "Sub-buffer should have 2 frames (got: {})", subBuffer.GetNFrames()));

  // Modify through sub-buffer
  subBuffer.FillWithSilence();

  // Verify modification affected the main buffer
  GOSoundBuffer::Item *mainData = buffer.GetData();
  GOAssert(
    mainData[2] == 0.0f && mainData[3] == 0.0f,
    "First channel of second frame should be zeroed");
  GOAssert(
    mainData[4] == 0.0f && mainData[5] == 0.0f,
    "Second channel of third frame should be zeroed");
}

void GOTestSoundBufferManaged::TestInvalidBufferOperations() {
  GOSoundBufferManaged buffer;

  // Operations on invalid buffer should be safe or assert
  AssertDimensions("InvalidBuffer", buffer, 0, 0);
  GOAssert(!buffer.GetData(), "Invalid buffer should have null data pointer");

  // Resize should work
  buffer.Resize(2, 3);
  AssertDimensions("After resize", buffer, 2, 3);
}

void GOTestSoundBufferManaged::run() {
  TestDefaultConstructor();
  TestConstructorWithDimensions();
  TestCopyConstructorFromBuffer();
  TestCopyConstructorFromManaged();
  TestMoveConstructor();
  TestCopyAssignmentFromBuffer();
  TestCopyAssignmentFromManaged();
  TestMoveAssignment();
  TestResize();
  TestResizeNoReallocation();
  TestSwap();
  TestComplexOperations();
  TestSubBufferCompatibility();
  TestInvalidBufferOperations();
}
