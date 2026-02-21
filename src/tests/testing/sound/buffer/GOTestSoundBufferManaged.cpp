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

void GOTestSoundBufferManaged::AssertMoveResult(
  const std::string &context,
  const GOSoundBuffer &buffer,
  const GOSoundBuffer::Item *originalPtr,
  const GOSoundBuffer &source,
  float offset) {
  GOAssert(buffer.isValid(), context + ": moved-to buffer should be valid");
  GOAssert(
    buffer.GetData() == originalPtr,
    context + ": moved-to buffer should have same data pointer");

  AssertSequentialData(context, buffer, offset);

  GOAssert(
    !source.isValid(), context + ": moved-from buffer should be invalid");
  GOAssert(
    !source.GetData(),
    context + ": moved-from buffer should have null data pointer");
}

void GOTestSoundBufferManaged::TestDeepCopy(
  const std::string &context,
  GOSoundBuffer::Item *sourceData,
  const GOSoundBuffer &buffer) {
  sourceData[0] = 999.0f;
  GOAssert(
    buffer.GetData()[0] != 999.0f,
    context + ": buffer should have independent copy of data");
}

void GOTestSoundBufferManaged::TestDefaultConstructor() {
  GOSoundBufferManaged buffer;

  AssertDimensions("DefaultConstructor", buffer, 0, 0);
}

void GOTestSoundBufferManaged::TestConstructorWithDimensions() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 3;

  GOSoundBufferManaged buffer(nChannels, nFrames);

  AssertDimensions("ConstructorWithDimensions", buffer, nChannels, nFrames);

  // Check that memory is accessible (no crash)
  fillWithSequential(buffer, 0.0f);
  AssertSequentialData("ConstructorWithDimensions", buffer, 0.0f);
}

void GOTestSoundBufferManaged::TestCopyConstructorFromBuffer() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 3;
  const unsigned nItems = nChannels * nFrames;

  std::vector<GOSoundBuffer::Item> sourceData(nItems);

  fillWithSequential(sourceData.data(), nItems, 1.0f);

  GOSoundBuffer sourceBuffer(sourceData.data(), nChannels, nFrames);
  GOSoundBufferManaged buffer(sourceBuffer);

  AssertDimensions("CopyConstructorFromBuffer", buffer, nChannels, nFrames);

  // Verify data was copied
  AssertSequentialData("CopyConstructorFromBuffer", buffer, 1.0f);

  // Verify it's a deep copy (modifying source doesn't affect buffer)
  TestDeepCopy("CopyConstructorFromBuffer", sourceData.data(), buffer);
}

void GOTestSoundBufferManaged::TestCopyConstructorFromManaged() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 3;

  GOSoundBufferManaged source(nChannels, nFrames);
  GOSoundBuffer::Item *sourceData = source.GetData();

  fillWithSequential(source, 1.0f);

  GOSoundBufferManaged buffer(source);

  AssertDimensions("CopyConstructorFromManaged", buffer, nChannels, nFrames);

  // Verify data was copied
  AssertSequentialData("CopyConstructorFromManaged", buffer, 1.0f);

  // Verify it's a deep copy
  TestDeepCopy("CopyConstructorFromManaged", sourceData, buffer);
}

void GOTestSoundBufferManaged::TestMoveConstructor() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 3;

  GOSoundBufferManaged source(nChannels, nFrames);
  GOSoundBuffer::Item *originalPtr = source.GetData();

  fillWithSequential(source, 1.0f);

  GOSoundBufferManaged buffer(std::move(source));

  AssertMoveResult("MoveConstructor", buffer, originalPtr, source, 1.0f);
}

void GOTestSoundBufferManaged::TestCopyAssignmentFromBuffer() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 3;
  const unsigned nItems = nChannels * nFrames;

  std::vector<GOSoundBuffer::Item> sourceData(nItems);

  fillWithSequential(sourceData.data(), nItems, 10.0f);

  GOSoundBuffer sourceBuffer(sourceData.data(), nChannels, nFrames);
  GOSoundBufferManaged buffer(1, 1); // Different size initially

  buffer = sourceBuffer;

  AssertDimensions("CopyAssignmentFromBuffer", buffer, nChannels, nFrames);

  // Verify data was copied
  AssertSequentialData("CopyAssignmentFromBuffer", buffer, 10.0f);
}

void GOTestSoundBufferManaged::TestCopyAssignmentFromManaged() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 3;

  GOSoundBufferManaged source(nChannels, nFrames);
  GOSoundBuffer::Item *sourceData = source.GetData();

  fillWithSequential(source, 20.0f);

  GOSoundBufferManaged buffer(1, 2); // Different size initially

  buffer = source;

  AssertDimensions("CopyAssignmentFromManaged", buffer, nChannels, nFrames);

  // Verify data was copied
  AssertSequentialData("CopyAssignmentFromManaged", buffer, 20.0f);

  // Verify it's a deep copy
  TestDeepCopy("CopyAssignmentFromManaged", sourceData, buffer);
}

void GOTestSoundBufferManaged::TestMoveAssignment() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 3;

  GOSoundBufferManaged source(nChannels, nFrames);
  GOSoundBuffer::Item *originalPtr = source.GetData();

  fillWithSequential(source, 30.0f);

  GOSoundBufferManaged buffer(1, 1); // Different size initially

  buffer = std::move(source);

  AssertMoveResult("MoveAssignment", buffer, originalPtr, source, 30.0f);
}

void GOTestSoundBufferManaged::TestResize() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 3;

  GOSoundBufferManaged buffer(nChannels, nFrames);

  fillWithSequential(buffer, 1.0f);

  GOSoundBuffer::Item *oldData = buffer.GetData();

  // Resize to larger
  buffer.Resize(2, 5);
  AssertDimensions("Resize to larger", buffer, 2, 5);
  GOAssert(
    buffer.GetData() != oldData,
    "Data pointer should change after resize to larger");

  // Resize to smaller
  oldData = buffer.GetData();
  buffer.Resize(1, 2);
  AssertDimensions("Resize to smaller", buffer, 1, 2);
  GOAssert(
    buffer.GetData() != oldData,
    "Data pointer should change after resize to different channel count");

  // Resize to zero
  buffer.Resize(0, 0);
  AssertDimensions("Resize to zero", buffer, 0, 0);
}

void GOTestSoundBufferManaged::TestResizeNoReallocation() {
  GOSoundBufferManaged buffer(2, 3);

  fillWithSequential(buffer, 1.0f);

  GOSoundBuffer::Item *oldData = buffer.GetData();

  // Resize to 3x2 (same total items: 2*3 = 3*2)
  buffer.Resize(3, 2);
  GOAssert(
    buffer.GetData() == oldData,
    "Data pointer should NOT change when total items unchanged");
  AssertDimensions("ResizeNoReallocation", buffer, 3, 2);

  // Data should be preserved
  AssertSequentialData("ResizeNoReallocation", buffer, 1.0f);
}

void GOTestSoundBufferManaged::TestSwap() {
  const unsigned nChannels1 = 2;
  const unsigned nFrames1 = 3;
  const unsigned nChannels2 = 3;
  const unsigned nFrames2 = 2;

  GOSoundBufferManaged buffer1(nChannels1, nFrames1);
  GOSoundBufferManaged buffer2(nChannels2, nFrames2);
  GOSoundBuffer::Item *data1 = buffer1.GetData();
  GOSoundBuffer::Item *data2 = buffer2.GetData();

  fillWithSequential(buffer1, 100.0f);
  fillWithSequential(buffer2, 200.0f);

  swap(buffer1, buffer2);

  // Check that dimensions are swapped
  AssertDimensions("Swap buffer1", buffer1, nChannels2, nFrames2);
  AssertDimensions("Swap buffer2", buffer2, nChannels1, nFrames1);

  // Check that data pointers are swapped
  GOAssert(
    buffer1.GetData() == data2,
    "Buffer1 should have buffer2's data after swap");
  GOAssert(
    buffer2.GetData() == data1,
    "Buffer2 should have buffer1's data after swap");

  // Check that data content is swapped
  AssertSequentialData("Swap buffer1", buffer1, 200.0f);
  AssertSequentialData("Swap buffer2", buffer2, 100.0f);
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
