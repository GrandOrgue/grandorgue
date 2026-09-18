/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundRecorderTask.h"

#include <format>
#include <vector>

#include <wx/file.h>
#include <wx/filename.h>

#include "sound/tasks/GOSoundBufferTaskBase.h"
#include "sound/tasks/GOSoundRecorderTask.h"

const std::string GOTestSoundRecorderTask::TEST_NAME
  = "GOTestSoundRecorderTask";

static constexpr unsigned N_SAMPLES_PER_BUFFER = 4;

namespace {

// A stub GOSoundBufferTaskBase input with a fixed, caller-chosen channel
// count - lets the test combine two outputs with different channel counts,
// the case ConvertData<T>()'s start_pos/m_Channels-stride logic exists for.
class StubBufferTask : public GOSoundBufferTaskBase {
public:
  StubBufferTask(unsigned nChannels)
    : GOSoundBufferTaskBase(
      PRIORITY_AUDIOGROUP, false, nChannels, N_SAMPLES_PER_BUFFER) {}

  bool DoRun(GOSchedulerThread *) override { return true; }
  void EnsureBufferReady(bool, GOSchedulerThread * = nullptr) override {}
};

// A distinct, small value per (stub index, channel, frame): float bit depth
// (m_BytesPerSample=4) uses the identity convertValue(), so what is written
// into the stub is exactly what must come back out of the WAV file.
float rampValue(unsigned stubI, unsigned channelI, unsigned frameI) {
  return 0.01f * static_cast<float>(stubI * 100 + channelI * 10 + frameI);
}

void fillWithRamp(GOSoundBufferTaskBase &task, unsigned stubI) {
  for (unsigned channelI = 0, nChannels = task.GetNChannels();
       channelI < nChannels;
       channelI++) {
    float *pData = task.GetChannelBuffer(channelI).GetData();

    for (unsigned frameI = 0; frameI < N_SAMPLES_PER_BUFFER; frameI++)
      pData[frameI] = rampValue(stubI, channelI, frameI);
  }
}

} // namespace

void GOTestSoundRecorderTask::TestGathersDistinctChannelCountsInOrder() {
  StubBufferTask stub0(1);
  StubBufferTask stub1(2);

  fillWithRamp(stub0, 0);
  fillWithRamp(stub1, 1);

  GOSoundRecorderTask recorder;

  recorder.SetSampleRate(44100);
  recorder.SetBytesPerSample(4); // float: convertValue() is the identity
  recorder.SetOutputs({&stub0, &stub1}, N_SAMPLES_PER_BUFFER);

  const wxString path = wxFileName::CreateTempFileName(wxT("goRecTest"));

  recorder.Open(path);
  GOAssert(recorder.IsOpen(), "recorder should be open after Open()");
  recorder.Run();
  recorder.Close();

  wxFile file(path);

  GOAssert(file.IsOpened(), "the recorded WAV file should be readable back");

  const unsigned nTotalChannels = 1 + 2;
  const unsigned nDataBytes
    = N_SAMPLES_PER_BUFFER * nTotalChannels * sizeof(float);
  const wxFileOffset fileLength = file.Length();

  GOAssert(
    fileLength
      == static_cast<wxFileOffset>(
        GOSoundRecorderTask::WAV_HEADER_SIZE + nDataBytes),
    "the WAV file should hold exactly the header plus one buffer's worth of "
    "data, not extra buffered rounds");
  file.Seek(GOSoundRecorderTask::WAV_HEADER_SIZE);

  std::vector<float> data(N_SAMPLES_PER_BUFFER * nTotalChannels);

  const ssize_t nRead = file.Read(data.data(), nDataBytes);

  file.Close();
  wxRemoveFile(path);

  GOAssert(
    nRead == static_cast<ssize_t>(nDataBytes),
    "the WAV data section should hold exactly one buffer's worth of "
    "interleaved float samples");

  // stub0 (1 channel) occupies channel slot 0 of every frame; stub1
  // (2 channels) occupies slots 1 and 2, in the order SetOutputs() listed
  // them - this is the exact gather/scatter path Stage 3's ConvertData<T>()
  // rewrite inverted from a raw interleaved read to a per-channel gather.
  for (unsigned frameI = 0; frameI < N_SAMPLES_PER_BUFFER; frameI++) {
    const unsigned frameBase = frameI * nTotalChannels;

    GOAssert(
      data[frameBase + 0] == rampValue(0, 0, frameI),
      std::format("frame {}: stub0 channel 0 mismatch", frameI));
    GOAssert(
      data[frameBase + 1] == rampValue(1, 0, frameI),
      std::format("frame {}: stub1 channel 0 mismatch", frameI));
    GOAssert(
      data[frameBase + 2] == rampValue(1, 1, frameI),
      std::format("frame {}: stub1 channel 1 mismatch", frameI));
  }
}

void GOTestSoundRecorderTask::run() {
  TestGathersDistinctChannelCountsInOrder();
}
