/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOWave.h"

#include <wx/file.h>
#include <wx/intl.h>
#include <wx/log.h>

#include "files/GOOpenedFile.h"

#include "GOWavPack.h"
#include "GOWavPackWriter.h"
#include "GOWaveTypes.h"

void GOWave::SetInvalid() {
  m_SampleData.free();
  m_Channels = 0;
  m_BytesPerSample = 0;
  m_SampleRate = 0;
  m_MidiNote = 0;
  m_PitchFract = 0;
  m_CuePoint = -1;
  m_hasRelease = false;
  m_isPacked = false;
  m_Loops.clear();
}

GOWave::GOWave() : m_Loops() {
  /* Start up the waveform in an invalid state */
  SetInvalid();
}

GOWave::~GOWave() {
  /* Close and free any currently open wave data */
  Close();
}

void GOWave::LoadFormatChunk(const uint8_t *ptr, unsigned long length) {
  /* FIXME: This could be done much more elequently */
  /* Ensure format chunk size is 16 (basic wave
   * format chunk... no extensible data... and
   * that the format tag is 1 */
  if (length < sizeof(GO_WAVEFORMATPCM))
    throw(wxString) _("< Invalid WAVE format chunk");

  GO_WAVEFORMATPCMEX *format = (GO_WAVEFORMATPCMEX *)ptr;
  unsigned formatCode = format->wf.wf.wFormatTag;
  if (formatCode != 1 && formatCode != 3)
    throw(wxString) _("< Unsupported PCM format");

  m_Channels = format->wf.wf.nChannels;
  m_SampleRate = format->wf.wf.nSamplesPerSec;

  unsigned bitsPerSample = format->wf.wBitsPerSample;
  if (bitsPerSample % 8)
    throw(wxString)
      _("< Bits per sample must be a multiple of 8 in this implementation");
  m_BytesPerSample = bitsPerSample / 8;

  if (formatCode == 3 && m_BytesPerSample != 4)
    throw(wxString) _("< Only 32bit IEEE float samples supported");
  else if (formatCode == 1 && m_BytesPerSample > 3)
    throw(wxString) _("< Unsupport PCM bit size");
}

void GOWave::LoadCueChunk(const uint8_t *ptr, unsigned long length) {
  if (length < sizeof(GO_WAVECUECHUNK))
    throw(wxString) _("< Invalid CUE chunk in");

  GO_WAVECUECHUNK *cue = (GO_WAVECUECHUNK *)ptr;
  unsigned nbPoints = cue->dwCuePoints;
  if (length < sizeof(GO_WAVECUECHUNK) + sizeof(GO_WAVECUEPOINT) * nbPoints)
    throw(wxString) _("< Invalid CUE chunk in");

  GO_WAVECUEPOINT *points = (GO_WAVECUEPOINT *)(ptr + sizeof(GO_WAVECUECHUNK));
  m_hasRelease = (nbPoints > 0);
  if (m_hasRelease)
    m_CuePoint = 0;
  for (unsigned k = 0; k < nbPoints; k++) {
    assert(m_Channels != 0);
    unsigned position = points[k].dwSampleOffset;
    if (position > m_CuePoint)
      m_CuePoint = position;
  }
}

void GOWave::LoadSamplerChunk(const uint8_t *ptr, unsigned long length) {
  if (length < sizeof(GO_WAVESAMPLERCHUNK))
    throw(wxString) _("< Invalid SMPL chunk in");

  GO_WAVESAMPLERCHUNK *sampler = (GO_WAVESAMPLERCHUNK *)ptr;
  unsigned numberOfLoops = sampler->cSampleLoops;
  if (
    length
    < sizeof(GO_WAVESAMPLERCHUNK) + sizeof(GO_WAVESAMPLERLOOP) * numberOfLoops)
    throw(wxString) _("<Invalid SMPL chunk in");

  GO_WAVESAMPLERLOOP *loops
    = (GO_WAVESAMPLERLOOP *)(ptr + sizeof(GO_WAVESAMPLERCHUNK));
  m_Loops.clear();
  for (unsigned k = 0; k < numberOfLoops; k++) {
    GOWaveLoop l;
    l.m_StartPosition = loops[k].dwStart;
    l.m_EndPosition = loops[k].dwEnd;
    m_Loops.push_back(l);
  }

  m_MidiNote = sampler->dwMIDIUnityNote;
  m_PitchFract = sampler->dwMIDIPitchFraction / (double)UINT_MAX * 100.0;
}

void GOWave::Open(GOOpenedFile *file) {
  const wxString fileName = file->GetName();

  if (!file->Open())
    throw wxString::Format(_("Failed to open file '%s'"), fileName);

  // Allocate memory for wave and read it.
  GOBuffer<uint8_t> content(file->GetSize());

  if (!file->Read(content))
    throw wxString::Format(_("Failed to read file '%s'"), fileName);

  file->Close();
  Open(content, fileName);
}

static void check_for_bounds(
  const wxString &fileName,
  GO_WAVECHUNKHEADER *pHeader,
  unsigned long offset,
  unsigned long length,
  unsigned long chunkOffset) {
  unsigned long size = pHeader->dwSize;

  if (offset + size > length) {
    char buf[5];

    strncpy(buf, (const char *)&pHeader->fccChunk, 4);
    buf[4] = '\x00';
    throw wxString::Format(
      _("Malformed wave file '%s'. The chunk '%4s' at %lu with the size 8 + %lu"
        " ends after the end of file %lu"),
      fileName,
      buf,
      chunkOffset,
      size,
      length);
  }
}

void GOWave::Open(const GOBuffer<uint8_t> &content, const wxString fileName) {
  /* Close any currently open wave data */
  Close();

  GOBuffer<uint8_t> buf;
  size_t offset = 0;
  size_t start = 0;
  size_t origDataLen = 0;
  try {
    if (content.GetSize() < 12)
      throw wxString::Format(_("Not a RIFF file: %s"), fileName);

    const uint8_t *ptr = content.get();
    unsigned length = content.GetSize();

    if (GOWavPack::IsWavPack(content)) {
      GOWavPack pack(content);
      if (!pack.Unpack())
        throw wxString::Format(
          _("Failed to decode WavePack data: %s"), fileName);

      m_SampleData = pack.GetSamples();

      buf = pack.GetWrapper();
      ptr = buf.get();
      length = buf.GetSize();
      offset = 0;
      m_isPacked = true;
      origDataLen = pack.GetOrigDataLen();
    }

    /* Read the header, get it's size and make sure that it makes sense. */
    GO_WAVECHUNKHEADER *riffHeader = (GO_WAVECHUNKHEADER *)(ptr + offset);
    unsigned long riffChunkSize = riffHeader->dwSize;

    /* Pribac compatibility */
    if (riffHeader->fccChunk != WAVE_TYPE_RIFF)
      throw wxString::Format(_("Invalid RIFF file: %s"), fileName);
    offset += sizeof(GO_WAVECHUNKHEADER);

    /* Make sure this is a RIFF/WAVE file */
    GO_WAVETYPEFIELD *riffIdent = (GO_WAVETYPEFIELD *)(ptr + offset);
    if (*riffIdent != WAVE_TYPE_WAVE)
      throw wxString::Format(_("Invalid RIFF/WAVE file: %s"), fileName);
    offset += sizeof(GO_WAVETYPEFIELD);

    if (m_isPacked) {
      if (riffChunkSize < origDataLen)
        throw wxString::Format(_("Inconsitant WavPack file: %s"), fileName);
      riffChunkSize -= origDataLen;
    }

    /* This is a bit more leaniant than the original code... it will
     * truncate the usable size of the file if the size on disk is larger
     * than the size of the RIFF chunk */
    if ((unsigned long)length > riffChunkSize + 8 + start)
      length = riffChunkSize + 8 + start;

    bool hasFormat = false;
    /* Find required chunks... */
    for (; offset + 8 <= length;) {
      /* Check for word alignment as per RIFF spec */
      assert((offset & 1) == 0);

      /* Read chunk header */
      GO_WAVECHUNKHEADER *header = (GO_WAVECHUNKHEADER *)(ptr + offset);
      unsigned long size = header->dwSize;
      unsigned long chunkOffset = offset;

      // skip the header
      offset += sizeof(GO_WAVECHUNKHEADER);

      if (header->fccChunk == WAVE_TYPE_DATA) {
        if (!hasFormat)
          throw wxString::Format(
            _("Malformed wave file '%s'. Format chunk must precede data "
              "chunk."),
            fileName);

        if (m_isPacked)
          size = 0;
        else {
          m_SampleData.free();
          check_for_bounds(fileName, header, offset, length, chunkOffset);
          m_SampleData.Append(ptr + offset, size);
        }
      }
      if (header->fccChunk == WAVE_TYPE_FMT) {
        hasFormat = true;
        check_for_bounds(fileName, header, offset, length, chunkOffset);
        LoadFormatChunk(ptr + offset, size);
      }
      if (header->fccChunk == WAVE_TYPE_CUE) { /* This used to only work if
                                                !load m_pipe_percussive[i] */
        check_for_bounds(fileName, header, offset, length, chunkOffset);
        LoadCueChunk(ptr + offset, size);
      }
      if (header->fccChunk == WAVE_TYPE_SAMPLE) { /* This used to only work if
                                                   !load m_pipe_percussive[i] */
        check_for_bounds(fileName, header, offset, length, chunkOffset);
        LoadSamplerChunk(ptr + offset, size);
      }
      /* Move to next chunk respecting word alignment
         Unless lack of final padding (non spec-compliant) */
      offset += size;
      if (offset < length)
        offset += size & 1;
    }

    if (offset != length)
      throw wxString::Format(_("Invalid WAV file: %s"), fileName);
    if (!m_SampleData.get() || !m_SampleData.GetSize())
      throw wxString::Format(_("No samples found: %s"), fileName);

    // learning lesson: never ever trust the range values of outside sources to
    // be correct!
    for (unsigned int i = 0; i < m_Loops.size(); i++) {
      if (
        (m_Loops[i].m_StartPosition >= m_Loops[i].m_EndPosition)
        || (m_Loops[i].m_StartPosition >= GetLength())
        || (m_Loops[i].m_EndPosition >= GetLength())
        || (m_Loops[i].m_EndPosition == 0)) {
        wxLogError(_("Invalid loop in the file: %s\n"), fileName);
        m_Loops.erase(m_Loops.begin() + i);
      }
    }
  } catch (...) {
    /* Free any memory that was allocated by chunk loading procedures */
    Close();

    /* Rethrow the exception */
    throw;
  }
}

void GOWave::Close() {
  /* Free the wave data if it has been alloc'ed */
  m_SampleData.free();

  /* Set the wave to the invalid state.  */
  SetInvalid();
}

unsigned GOWave::GetBitsPerSample() const { return m_BytesPerSample * 8; }

unsigned GOWave::GetChannels() const { return m_Channels; }

bool GOWave::HasReleaseMarker() const { return m_hasRelease; }

unsigned GOWave::GetReleaseMarkerPosition() const {
  /* release = dwSampleOffset from cue chunk. This is a byte offset into
   * the data chunk. Compute this to a block start
   */
  return m_CuePoint;
}

const GOWaveLoop &GOWave::GetLongestLoop() const {
  if (m_Loops.size() < 1)
    throw(wxString) _("wave does not contain loops");

  assert(m_Loops[0].m_EndPosition > m_Loops[0].m_StartPosition);
  unsigned lidx = 0;
  for (unsigned int i = 1; i < m_Loops.size(); i++) {
    assert(m_Loops[i].m_EndPosition > m_Loops[i].m_StartPosition);
    if (
      (m_Loops[i].m_EndPosition - m_Loops[i].m_StartPosition)
      > (m_Loops[lidx].m_EndPosition - m_Loops[lidx].m_StartPosition))
      lidx = i;
  }

  return m_Loops[lidx];
}

unsigned GOWave::GetLength() const {
  if (m_isPacked)
    return m_SampleData.GetSize() / (4 * m_Channels);
  /* return number of samples in the stream */
  assert((m_SampleData.GetSize() % (m_BytesPerSample * m_Channels)) == 0);
  return m_SampleData.GetSize() / (m_BytesPerSample * m_Channels);
}

template <class T> void GOWave::writeNext(uint8_t *&output, const T &value) {
  *(T *)output = value;
  output += sizeof(T);
}

template <class T> T GOWave::readNext(const uint8_t *&input) {
  T val = *(T *)input;
  input += sizeof(T);
  return val;
}

void GOWave::ReadSamples(
  void *dest_buffer /** Pointer to received sample data */
  ,
  GOWave::SAMPLE_FORMAT read_format /** Format of the above buffer */
  ,
  unsigned sample_rate /** Sample rate to read data at */
  ,
  int return_channels /** number of channels to return or if negative,
                         specific channel as mono*/
) const {
  if (m_SampleRate != sample_rate)
    throw(wxString) _("bad format!");

  if (m_BytesPerSample < 1 || m_BytesPerSample > 4)
    throw(wxString) _("Unsupported format");

  unsigned select_channel = 0;
  if (return_channels < 0) {
    if ((unsigned)-return_channels > m_Channels)
      throw(wxString) _("Unsupported channel number");
    select_channel = -return_channels;
  } else if (m_Channels != (unsigned)return_channels && return_channels != 1)
    throw(wxString) _("Unsupported channel count");

  unsigned merge_count = 1;
  /* need reduce stereo to mono ? */
  if (m_Channels != (unsigned)return_channels && return_channels == 1)
    merge_count = m_Channels;
  if (select_channel != 0)
    merge_count = m_Channels;

  const uint8_t *input = m_SampleData.get();
  uint8_t *output = (uint8_t *)dest_buffer;

  unsigned len = m_Channels * GetLength() / merge_count;
  for (unsigned i = 0; i < len; i++) {
    int value
      = 0; /* Value will be stored with 24 fractional bits of precision */
    for (unsigned j = 0; j < merge_count; j++) {
      int val; /* Value will be stored with 24 fractional bits of precision */
      if (m_isPacked && m_BytesPerSample != 4) {
        val = readNext<int32_t>(input);
        switch (m_BytesPerSample) {
        case 1:
          val <<= 16;
          break;
        case 2:
          val <<= 8;
          break;
        }
      } else {
        switch (m_BytesPerSample) {
        case 1:
          val = readNext<GOInt8>(input) - 0x80;
          val <<= 16;
          break;
        case 2:
          val = readNext<GOInt16LE>(input);
          val <<= 8;
          break;
        case 3:
          val = readNext<GOInt24LE>(input);
          break;
        case 4:
          val = readNext<float>(input) * (float)(1 << 23);
          break;
        default:
          throw(wxString) _("bad format!");
        }
      }

      if (select_channel && select_channel != j + 1)
        continue;
      value += val;
    }
    if (select_channel == 0 && merge_count > 1)
      value = value / (int)merge_count;

    switch (read_format) {
    case SF_SIGNEDBYTE_8:
      writeNext<GOInt8>(output, value >> 16);
      break;
    case SF_SIGNEDSHORT_9:
      writeNext<GOInt16>(output, value >> 15);
      break;
    case SF_SIGNEDSHORT_10:
      writeNext<GOInt16>(output, value >> 14);
      break;
    case SF_SIGNEDSHORT_11:
      writeNext<GOInt16>(output, value >> 13);
      break;
    case SF_SIGNEDSHORT_12:
      writeNext<GOInt16>(output, value >> 12);
      break;
    case SF_SIGNEDSHORT_13:
      writeNext<GOInt16>(output, value >> 11);
      break;
    case SF_SIGNEDSHORT_14:
      writeNext<GOInt16>(output, value >> 10);
      break;
    case SF_SIGNEDSHORT_15:
      writeNext<GOInt16>(output, value >> 9);
      break;
    case SF_SIGNEDSHORT_16:
      writeNext<GOInt16>(output, value >> 8);
      break;
    case SF_SIGNEDINT24_17:
      writeNext<GOInt24>(output, value >> 7);
      break;
    case SF_SIGNEDINT24_18:
      writeNext<GOInt24>(output, value >> 6);
      break;
    case SF_SIGNEDINT24_19:
      writeNext<GOInt24>(output, value >> 5);
      break;
    case SF_SIGNEDINT24_20:
      writeNext<GOInt24>(output, value >> 4);
      break;
    case SF_SIGNEDINT24_21:
      writeNext<GOInt24>(output, value >> 3);
      break;
    case SF_SIGNEDINT24_22:
      writeNext<GOInt24>(output, value >> 2);
      break;
    case SF_SIGNEDINT24_23:
      writeNext<GOInt24>(output, value >> 1);
      break;
    case SF_SIGNEDINT24_24:
      writeNext<GOInt24>(output, value);
      break;
    case SF_IEEE_FLOAT:
      writeNext<float>(output, value / (float)(1 << 23));
      break;
    default:
      throw(wxString) _("bad return format!");
    }
  }
}

unsigned GOWave::GetNbLoops() const { return m_Loops.size(); }

const GOWaveLoop &GOWave::GetLoop(unsigned idx) const {
  assert(idx < m_Loops.size());
  return m_Loops[idx];
}

unsigned GOWave::GetSampleRate() const { return m_SampleRate; }

unsigned GOWave::GetMidiNote() const { return m_MidiNote; }

float GOWave::GetPitchFract() const { return m_PitchFract; }

bool GOWave::IsWave(const GOBuffer<uint8_t> &data) {
  if (data.GetSize() < 12)
    return false;
  GO_WAVECHUNKHEADER *riffHeader = (GO_WAVECHUNKHEADER *)data.get();
  if (riffHeader->fccChunk != WAVE_TYPE_RIFF)
    return false;
  GO_WAVETYPEFIELD *riffIdent
    = (GO_WAVETYPEFIELD *)(data.get() + sizeof(GO_WAVECHUNKHEADER));
  return *riffIdent == WAVE_TYPE_WAVE;
}

bool GOWave::IsWaveFile(const GOBuffer<uint8_t> &data) {
  return IsWave(data) || GOWavPack::IsWavPack(data);
}

bool GOWave::Save(GOBuffer<uint8_t> &buf) {
  GOBuffer<uint8_t> header(sizeof(GO_WAVECHUNKHEADER));
  GO_WAVETYPEFIELD wav = WAVE_TYPE_WAVE;
  header.Append((const uint8_t *)&wav, sizeof(wav));
  GO_WAVECHUNKHEADER head;
  GO_WAVEFORMATPCM fmt;
  fmt.wf.wFormatTag = m_BytesPerSample == 4 ? 3 : 1;
  fmt.wf.nChannels = GetChannels();
  fmt.wf.nSamplesPerSec = GetSampleRate();
  fmt.wf.nAvgBytesPerSec = GetSampleRate() * GetChannels() * m_BytesPerSample;
  fmt.wf.nBlockAlign = GetChannels() * m_BytesPerSample;
  fmt.wBitsPerSample = GetBitsPerSample();
  head.fccChunk = WAVE_TYPE_FMT;
  head.dwSize = sizeof(fmt);
  header.Append((const uint8_t *)&head, sizeof(head));
  header.Append((const uint8_t *)&fmt, sizeof(fmt));

  if (m_CuePoint != (unsigned)-1) {
    GO_WAVECUECHUNK cue;
    cue.dwCuePoints = 1;

    GO_WAVECUEPOINT point;
    point.dwName = 1;
    point.dwPosition = 0;
    point.fccChunk = WAVE_TYPE_DATA;
    point.dwChunkStart = 0;
    point.dwBlockStart = 0;
    point.dwSampleOffset = m_CuePoint;

    head.fccChunk = WAVE_TYPE_CUE;
    head.dwSize = sizeof(cue) + sizeof(point);

    header.Append((const uint8_t *)&head, sizeof(head));
    header.Append((const uint8_t *)&cue, sizeof(cue));
    header.Append((const uint8_t *)&point, sizeof(point));
  }
  if (m_MidiNote != 0 || m_PitchFract != 0 || m_Loops.size() > 0) {
    GO_WAVESAMPLERLOOP loop;
    GOBuffer<uint8_t> loops;
    for (unsigned i = 0; i < m_Loops.size(); i++) {
      loop.dwIdentifier = i;
      loop.dwType = 0;
      loop.dwStart = m_Loops[i].m_StartPosition;
      loop.dwEnd = m_Loops[i].m_EndPosition;
      loop.dwFraction = 0;
      loop.dwPlayCount = 0;
      loops.Append((const uint8_t *)&loop, sizeof(loop));
    }
    GO_WAVESAMPLERCHUNK smpl;
    smpl.dwManufacturer = 0;
    smpl.dwProduct = 0;
    smpl.dwSamplePeriod = 1000000000.0 / m_SampleRate;
    smpl.dwMIDIUnityNote = m_MidiNote;
    smpl.dwMIDIPitchFraction = m_PitchFract / 100.0 * UINT_MAX;
    smpl.dwSMPTEFormat = 0;
    smpl.dwSMPTEOffset = 0;
    smpl.cSampleLoops = m_Loops.size();
    smpl.cbSamplerData = 0;
    head.fccChunk = WAVE_TYPE_SAMPLE;
    head.dwSize = sizeof(smpl) + loops.GetSize();
    header.Append((const uint8_t *)&head, sizeof(head));
    header.Append((const uint8_t *)&smpl, sizeof(smpl));
    header.Append(loops);
  }

  head.fccChunk = WAVE_TYPE_DATA;
  head.dwSize = m_BytesPerSample * GetChannels() * GetLength();
  header.Append((const uint8_t *)&head, sizeof(head));

  GO_WAVECHUNKHEADER *start = (GO_WAVECHUNKHEADER *)header.get();
  start->fccChunk = WAVE_TYPE_RIFF;
  start->dwSize = header.GetSize() - sizeof(GO_WAVECHUNKHEADER)
    + GetLength() * m_BytesPerSample * GetChannels();

  GOBuffer<int32_t> data(GetLength() * GetChannels());
  if (m_isPacked)
    memcpy(data.get(), m_SampleData.get(), data.GetSize());
  else {
    const uint8_t *input = m_SampleData.get();
    for (unsigned i = 0; i < GetLength() * GetChannels(); i++) {
      int32_t val;
      switch (m_BytesPerSample) {
      case 1:
        val = readNext<GOInt8>(input) - 0x80;
        break;
      case 2:
        val = readNext<GOInt16LE>(input);
        break;
      case 3:
        val = readNext<GOInt24LE>(input);
        break;
      case 4:
        val = readNext<int32_t>(input);
        break;
      default:
        throw(wxString) _("bad format!");
      }
      data[i] = val;
    }
  }

  GOWavPackWriter pack;
  if (!pack.Init(
        GetChannels(),
        GetBitsPerSample(),
        m_BytesPerSample,
        GetSampleRate(),
        GetLength()))
    return false;
  if (!pack.AddWrapper(header))
    return false;
  if (!pack.AddSampleData(data))
    return false;
  return pack.GetResult(buf);
}
