/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
 *
 * MyOrgan 1.0.6 Codebase - Copyright 2006 Milan Digital Audio LLC
 * MyOrgan is a Trademark of Milan Digital Audio LLC
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "GOSoundEngine.h"
#include "GOSoundProvider.h"
#include "GOrgueEvent.h"
#include "GOrguePipe.h"
#include "GOrgueReleaseAlignTable.h"
#include "GOrgueWindchest.h"
#include "GrandOrgueFile.h"

GOSoundEngine::GOSoundEngine() :
	m_PolyphonyLimiting(true),
	m_ScaledReleases(true),
	m_ReleaseAlignmentEnabled(true),
	m_Volume(100),
	m_SampleRate(0),
	m_CurrentTime(0),
	m_SamplerPool(),
	m_DetachedRelease(1),
	m_Windchests(),
	m_Tremulants()
{
	m_SamplerPool.SetUsageLimit(2048);
	m_PolyphonySoftLimit = (m_SamplerPool.GetUsageLimit() * 3) / 4;
	Reset();
}

void GOSoundEngine::Reset()
{
	for (unsigned i = 0; i < m_Windchests.size(); i++)
	{
		m_Windchests[i].sampler = 0;
		m_Windchests[i].count = 0;
	}
	for (unsigned i = 0; i < m_Tremulants.size(); i++)
	{
		m_Tremulants[i].sampler = 0;
		m_Tremulants[i].count = 0;
	}
	for (unsigned i = 0; i < m_DetachedRelease.size(); i++)
	{
		m_DetachedRelease[i].sampler = 0;
		m_DetachedRelease[i].count = 0;
	}
	m_SamplerPool.ReturnAll();
	m_CurrentTime = 0;
	ResetDoneFlags();
}

void GOSoundEngine::SetVolume(int volume)
{
	m_Volume = volume;
}

void GOSoundEngine::SetSampleRate(unsigned sample_rate)
{
	m_SampleRate = sample_rate;
}

unsigned GOSoundEngine::GetSampleRate()
{
	return m_SampleRate;
}

void GOSoundEngine::SetHardPolyphony(unsigned polyphony)
{
	m_SamplerPool.SetUsageLimit(polyphony);
	m_PolyphonySoftLimit = (m_SamplerPool.GetUsageLimit() * 3) / 4;
}

void GOSoundEngine::SetPolyphonyLimiting(bool limiting)
{
	m_PolyphonyLimiting = limiting;
}

unsigned GOSoundEngine::GetHardPolyphony() const
{
	return m_SamplerPool.GetUsageLimit();
}

int GOSoundEngine::GetVolume() const
{
	return m_Volume;
}

void GOSoundEngine::SetScaledReleases(bool enable)
{
	m_ScaledReleases = enable;
}

void GOSoundEngine::StartSampler(GO_SAMPLER* sampler, int sampler_group_id)
{
	GOSamplerEntry* state;

	if (sampler_group_id == 0)
		state = &m_DetachedRelease[0];
	else if (sampler_group_id < 0)
		state = &m_Tremulants[-1-sampler_group_id];
	else if (sampler_group_id > (int) m_Windchests.size())
		state = &m_DetachedRelease[sampler_group_id - m_Windchests.size()];
	else
		state = &m_Windchests[sampler_group_id - 1];

	wxCriticalSectionLocker locker(state->lock);
	sampler->sampler_group_id = sampler_group_id;
	sampler->next = state->sampler;
	state->sampler = sampler;
	state->count++;
}

void GOSoundEngine::StartSamplerUnlocked(GO_SAMPLER* sampler, int sampler_group_id)
{
	GOSamplerEntry* state;

	if (sampler_group_id == 0)
		state = &m_DetachedRelease[0];
	else if (sampler_group_id < 0)
		state = &m_Tremulants[-1-sampler_group_id];
	else if (sampler_group_id > (int) m_Windchests.size())
		state = &m_DetachedRelease[sampler_group_id - m_Windchests.size()];
	else
		state = &m_Windchests[sampler_group_id - 1];

	sampler->sampler_group_id = sampler_group_id;
	sampler->next = state->sampler;
	state->sampler = sampler;
	state->count++;
}

void GOSoundEngine::Setup(GrandOrgueFile* organ_file, unsigned release_count)
{
	m_Windchests.resize(organ_file->GetWinchestGroupCount());
	if (release_count < 1)
		release_count = 1;
	m_DetachedRelease.resize(release_count);
	for (unsigned i = 0; i < m_Windchests.size(); i++)
		m_Windchests[i].windchest = organ_file->GetWindchest(i);
	m_Tremulants.resize(organ_file->GetTremulantCount());
	Reset();
}

typedef wxInt16 steroSample[0][2];

static
inline
void stereoUncompressed
	(GO_SAMPLER* sampler
	,int* output
	)
{

	// "borrow" the output buffer to compute release alignment info
	steroSample& input = (steroSample&)*(wxInt16*)(sampler->pipe_section->data);

	// copy the sample buffer
	for (unsigned int i = 0; i < BLOCKS_PER_FRAME; sampler->position += sampler->increment, output+=2, i++)
	{
		unsigned pos = (unsigned)sampler->position;
		float fract = sampler->position - pos;
		output[0] = input[pos][0] * (1 - fract) + input[pos + 1][0] * fract;
		output[1] = input[pos][1] * (1 - fract) + input[pos + 1][1] * fract;
	}

	// update sample history (for release alignment / compression)
	unsigned pos = (unsigned)sampler->position;
	for (unsigned i = BLOCK_HISTORY; i > 0 && pos; i--, pos--)
	{
		sampler->history[i - 1][0] = input[pos][0];
		sampler->history[i - 1][1] = input[pos][1];
	}
}

static
inline
void monoUncompressed
	(GO_SAMPLER* sampler
	,int* output
	)
{

	// copy the sample buffer
	wxInt16* input = (wxInt16*)(sampler->pipe_section->data);
	for (unsigned int i = 0; i < BLOCKS_PER_FRAME; i++, sampler->position += sampler->increment, output += 2)
	{
		unsigned pos = (unsigned)sampler->position;
		float fract = sampler->position - pos;
		output[0] = input[pos] * (1 - fract) + input[pos + 1] * fract;
		output[1] = input[pos] * (1 - fract) + input[pos + 1] * fract;
	}

	// update sample history (for release alignment / compression)
	unsigned pos = (unsigned)sampler->position;
	for (unsigned i = BLOCK_HISTORY; i > 0 && pos; i--, pos--)
		sampler->history[i - 1][0] = input[pos];
}

static
inline
void monoCompressed
	(GO_SAMPLER* sampler
	,int* output
	)
{

	throw (wxString)_("unimplemented");

    /*short* v=(short*)&sampler->v;
    short* f=(short*)&sampler->f;
    wxByte* input=sampler->ptr+sampler->position;

    // we are in little endian, so the byte the most to the right is in input[0]
    // check at the end of the first 32bits
    if(input[0]&0x01)
        // an int is 32 bit so we will use int
    {
        int inputAsInt=*(int*)input;
        inputAsInt>>=1;
        v[0]+=((char*)(&inputAsInt))[0];
        v[1]+=((char*)(&inputAsInt))[0];
        v[2]+=((char*)(&inputAsInt))[1];
        v[3]+=((char*)(&inputAsInt))[1];
        sampler->position+=4;

    }
    else
    {
        int inputAsInt1=*(int*)input;
        inputAsInt1>>=1;

        v[0]+=(((char*)(&inputAsInt1))[0]<<8)|((short)(input[2]));
        v[1]+=(((char*)(&inputAsInt1))[0]<<8)|((short)(input[2]));
        v[2]+=(((char*)(&inputAsInt1))[1]<<8)|((short)(input[3]));
        v[3]+=(((char*)(&inputAsInt1))[1]<<8)|((short)(input[3]));
        sampler->position+=8;
    }

    f[0]+=v[0];
    f[1]+=v[1];
    f[2]+=v[2];
    f[3]+=v[3];


    output[0] = f[0];
    output[1] = f[1];
    output[2] = f[2];
    output[3] = f[3];*/

}

//if the data is compressed, 32 bits represents 4 data ((Right and Left) * 2)
// we know the data is compressed because these 32bits ends with 1.
//if the data is uncompressed, 64 bits represents 4 data
// the first 32bits end with a 0.
static
inline
void stereoCompressed
	(GO_SAMPLER* sampler
	,int* output
	)
{

	throw (wxString)_("unimplemented");

/*throw 0;

	short* v=(short*)&sampler->v;
	short* f=(short*)&sampler->f;
	wxByte* input=sampler->ptr+sampler->position;

	// we are in little endian, so the byte the most to the right is in input[0]
	// check at the end of the first 32bits
	if(input[0]&0x01)
	{
		// an int is 32 bit so we will use int
		int inputAsInt=*(int*)input;
		inputAsInt>>=1;
		v[0]+=((char*)(&inputAsInt))[0];
		v[1]+=((char*)(&inputAsInt))[1];
		v[2]+=((char*)(&inputAsInt))[2];
		v[3]+=((char*)(&inputAsInt))[3];
		sampler->position+=4;

	}
	else
	{
		int inputAsInt1=*(int*)input;
		inputAsInt1>>=1;

		v[0]+=(((char*)(&inputAsInt1))[0]<<8)|((short)(input[4]));
		v[1]+=(((char*)(&inputAsInt1))[1]<<8)|((short)(input[5]));
		v[2]+=(((char*)(&inputAsInt1))[2]<<8)|((short)(input[6]));
		v[3]+=(((char*)(&inputAsInt1))[3]<<8)|((short)(input[7]));
		sampler->position+=8;
	}

	f[0]+=v[0];
	f[1]+=v[1];
	f[2]+=v[2];
	f[3]+=v[3];


	output[0] = f[0];
    output[1] = f[1];
    output[2] = f[2];
    output[3] = f[3];
*/
}

static
inline
void GetNextFrame
	(GO_SAMPLER* sampler
	,int* buffer
	)
{

	switch (sampler->pipe_section->type)
	{
		case AC_COMPRESSED_STEREO:
			stereoCompressed(sampler, buffer);
			break;
		case AC_UNCOMPRESSED_STEREO:
			stereoUncompressed(sampler, buffer);
			break;
		case AC_COMPRESSED_MONO:
			monoCompressed(sampler, buffer);
			break;
		default:
			assert(sampler->pipe_section->type == AC_UNCOMPRESSED_MONO);
			monoUncompressed(sampler, buffer);
	}

}

static
inline
void ApplySamplerFade
	(GO_SAMPLER* sampler
	,unsigned int n_blocks
	,int* decoded_sampler_audio_frame
	)
{

	/* Multiply each of the buffer samples by the fade factor - note:
	 * FADE IS NEGATIVE. A positive fade would indicate a gain of zero.
	 * Note: this for loop has been split by an if to aide the vectorizer.
	 */
	int fade_in_plus_out = sampler->fadein + sampler->fadeout;
	int fade = sampler->fade;
	if (fade_in_plus_out)
	{

		for(unsigned int i = 0; i < n_blocks / 2; i++, decoded_sampler_audio_frame += 4)
		{

			decoded_sampler_audio_frame[0] *= fade;
			decoded_sampler_audio_frame[1] *= fade;
			decoded_sampler_audio_frame[2] *= fade;
			decoded_sampler_audio_frame[3] *= fade;

			fade += fade_in_plus_out;
			if (fade > 0)
			{
				fade = 0;
				sampler->fadeout = 0;
			}
			else if (fade < sampler->fademax)
			{
				fade = sampler->fademax;
				sampler->fadein = 0;
			}

		}

		sampler->fade = fade;

	}
	else
	{

		for(unsigned int i = 0; i < n_blocks / 2; i++, decoded_sampler_audio_frame += 4)
		{

			decoded_sampler_audio_frame[0] *= fade;
			decoded_sampler_audio_frame[1] *= fade;
			decoded_sampler_audio_frame[2] *= fade;
			decoded_sampler_audio_frame[3] *= fade;

		}

	}

	if (sampler->pipe)
	{
		if (sampler->fadein < 0)
		{
			if (sampler->faderemain >= n_blocks)
				sampler->faderemain -= n_blocks;
			else
				sampler->fadein = 0;
		}
	}

}

inline
void GOSoundEngine::ReadSamplerFrames
	(GO_SAMPLER* sampler
	,unsigned int n_blocks
	,int* decoded_sampler_audio_frame
	)
{

	for(unsigned int i = 0; i < n_blocks; i += BLOCKS_PER_FRAME, decoded_sampler_audio_frame += BLOCKS_PER_FRAME * 2)
	{

		if (!sampler->pipe)
		{
			std::fill
				(decoded_sampler_audio_frame
				,decoded_sampler_audio_frame + (BLOCKS_PER_FRAME * 2)
				,0
				);
			continue;
		}

		GetNextFrame(sampler, decoded_sampler_audio_frame);

		if(sampler->pipe_section->stage == GSS_RELEASE)
		{

			/* If this is the end of the release, and there are no more
			 * samples to play back, the sampler is no-longer needed.
			 * We can set the pipe to NULL and break out of this loop.
			 */
			assert(sampler->pipe);
			if (sampler->position >= sampler->pipe->GetRelease()->sample_count)
				sampler->pipe = NULL;

		}
		else
		{
			unsigned currentBlockSize = sampler->pipe_section->sample_count;
			if(sampler->position >= currentBlockSize)
			{
				sampler->pipe_section = sampler->pipe->GetLoop();
				if (sampler->pipe_section->data == NULL)
				{
					/* the pipe is percussive and the attack has completed
					 * so we are therefore finished with this sampler. */
					sampler->pipe = NULL;
				}
				else
				{
					/* the pipe is not percussive (normal). The loop or
					 * attack segment has completed so we now (re)enter the
					 * loop. */
					sampler->position -= currentBlockSize;
					sampler->increment = sampler->pipe_section->sample_rate / (float) m_SampleRate;
				}
			}
		}

	}

}

void GOSoundEngine::ProcessAudioSamplers (GOSamplerEntry& state, unsigned int n_frames, int* output_buffer)
{
	wxCriticalSectionLocker locker(state.lock);
	GO_SAMPLER** list_start = &state.sampler;

	assert((n_frames & (n_frames - 1)) == 0);
	assert(n_frames > BLOCKS_PER_FRAME);
	GO_SAMPLER* previous_valid_sampler = *list_start;
	for (GO_SAMPLER* sampler = *list_start; sampler; sampler = sampler->next)
	{

		const bool process_sampler = (sampler->time <= m_CurrentTime);
		if (process_sampler)
		{

			if  (
					(m_PolyphonyLimiting) &&
					(m_SamplerPool.UsedSamplerCount() >= m_PolyphonySoftLimit) &&
					(sampler->pipe_section->stage == GSS_RELEASE) &&
					(m_CurrentTime - sampler->time > 172)
				)
				sampler->fadeout = 4;

			ReadSamplerFrames
				(sampler
				,n_frames
				,state.temp
				);

			ApplySamplerFade
				(sampler
				,n_frames
				,state.temp
				);

			if (sampler->stop)
			{
				CreateReleaseSampler(sampler);

				/* The above code created a new sampler to playback the release, the
				 * following code takes the active sampler for this pipe (which will be
				 * in either the attack or loop section) and sets the fadeout property
				 * which will decay this portion of the pipe. The sampler will
				 * automatically be placed back in the pool when the fade restores to
				 * zero.
				 *
				 * Fadeout is added to the fade value in the sampler every 2 samples, so
				 * a pipe with an amplitude of 10000 (unadjusted playback level) will have
				 * a fadeout value of
				 *
				 *   (32768 - 128) >> 8
				 * = 127
				 *
				 * Which means that the sampler will fade out over a period of
				 *   (2 * 32768) / 127
				 * = 516 samples = 12ms
				 */
				sampler->fadeout = (-sampler->pipe->GetScaleAmplitude() - 128) >> 8; /* recall that ra_amp is negative
												      * so this will actually be a
												      * positive number */
				if (!sampler->fadeout) /* ensure that the sample will fade out */
					sampler->fadeout++;

				sampler->stop = false;
			}

			/* Add these samples to the current output buffer shifting
			 * right by the necessary amount to bring the sample gain back
			 * to unity (this value is computed in GOrguePipe.cpp)
			 */
			int shift = sampler->shift;
			int* write_iterator = output_buffer;
			int* decode_pos = state.temp;
			for(unsigned int i = 0; i < n_frames / 2; i++, write_iterator += 4, decode_pos += 4)
			{
				write_iterator[0] += decode_pos[0] >> shift;
				write_iterator[1] += decode_pos[1] >> shift;
				write_iterator[2] += decode_pos[2] >> shift;
				write_iterator[3] += decode_pos[3] >> shift;
			}

		}

		/* if this sampler's pipe has been set to null or the fade value is
		 * zero, the sample is no longer required and can be removed from the
		 * linked list. If it was still supplying audio, we must update the
		 * previous valid sampler. */
		if (!sampler->pipe || (!sampler->fade && process_sampler))
		{
			/* sampler needs to be removed from the list */
			if (sampler == *list_start)
				*list_start = sampler->next;
			else
				previous_valid_sampler->next = sampler->next;
			m_SamplerPool.ReturnSampler(sampler);
			state.count--;
		}
		else
			previous_valid_sampler = sampler;

	}
	state.done = true;
}

void GOSoundEngine::Process(unsigned sampler_group_id, unsigned n_frames)
{
	GOSamplerEntry* state;

	if (sampler_group_id == 0)
		state = &m_DetachedRelease[0];
	else if (sampler_group_id > m_Windchests.size())
		state = &m_DetachedRelease[sampler_group_id - m_Windchests.size()];
	else
		state = &m_Windchests[sampler_group_id - 1];

	wxCriticalSectionLocker locker(state->mutex);

	int* this_buff = state->buff;

	if (state->done)
		return;

	std::fill(this_buff, this_buff + GO_SOUND_BUFFER_SIZE, 0);

	ProcessAudioSamplers(*state, n_frames, this_buff);
}

void GOSoundEngine::ResetDoneFlags()
{
	for (unsigned j = 0; j < m_Windchests.size(); j++)
	{
		wxCriticalSectionLocker locker(m_Windchests[j].mutex);
		m_Windchests[j].done = false;
	}
	for (unsigned j = 0; j < m_DetachedRelease.size(); j++)
	{
		wxCriticalSectionLocker locker(m_DetachedRelease[j].mutex);
		m_DetachedRelease[j].done = false;
	}
}

// this callback will fill the buffer with bufferSize frame
// audio is opened with 32 bit stereo, so one frame is 64bit (32bit for right 32bit for left)
// So buffer can handle 8*bufferSize char (or 2*bufferSize float)
int GOSoundEngine::GetSamples
	(float      *output_buffer
	,unsigned    n_frames
	,double      stream_time
	,METER_INFO *meter_info
	)
{

	/* if no samplers playing, or sound is disabled, fill buffer with zero */
	if (!m_SamplerPool.UsedSamplerCount())
		memset(output_buffer, 0, (n_frames * sizeof(float)));

	/* initialise the output buffer */
	std::fill(m_FinalBuffer, m_FinalBuffer + GO_SOUND_BUFFER_SIZE, 0);

	for (unsigned j = 0; j < m_Tremulants.size(); j++)
	{
		if (m_Tremulants[j].sampler == NULL)
			continue;

		int* this_buff = m_Tremulants[j].buff;

		std::fill(this_buff, this_buff + GO_SOUND_BUFFER_SIZE, 0x800000);

		ProcessAudioSamplers (m_Tremulants[j], n_frames, this_buff);
	}

	for (unsigned j = 0; j < m_Windchests.size(); j++)
	{

		if (m_Windchests[j].sampler == NULL)
			continue;

		int* this_buff = m_Windchests[j].buff;

		wxCriticalSectionLocker locker(m_Windchests[j].mutex);

		if (!m_Windchests[j].done)
		{
			std::fill(this_buff, this_buff + GO_SOUND_BUFFER_SIZE, 0);

			ProcessAudioSamplers(m_Windchests[j], n_frames, this_buff);
		}

		GOrgueWindchest* current_windchest = m_Windchests[j].windchest;
		double d = current_windchest->GetVolume();
		d *= m_Volume;
		d *= 0.00000000059604644775390625;  // (2 ^ -24) / 100
		float f = d;
		std::fill(m_VolumeBuffer, m_VolumeBuffer + GO_SOUND_BUFFER_SIZE, f);

		for (unsigned i = 0; i < current_windchest->GetTremulantCount(); i++)
		{
			unsigned tremulant_pos = current_windchest->GetTremulantId(i);
			if (!m_Tremulants[tremulant_pos].sampler)
				continue;
			int *ptr = m_Tremulants[tremulant_pos].buff;
			for (unsigned int k = 0; k < n_frames*2; k++)
			{
				//multiply by 2^-23
				m_VolumeBuffer[k] *= ldexp(ptr[k], -23);
			}
		}

		for (unsigned int k = 0; k < n_frames*2; k++)
		{
			double d = this_buff[k];
			d *= m_VolumeBuffer[k];
			m_FinalBuffer[k] += d;
		}

	}

	for (unsigned j = 0; j < m_DetachedRelease.size(); j++)
	{
		if (m_DetachedRelease[j].sampler == NULL)
			continue;

		int* this_buff = m_DetachedRelease[j].buff;

		wxCriticalSectionLocker locker(m_DetachedRelease[j].mutex);

		if (!m_DetachedRelease[j].done)
		{
			std::fill(this_buff, this_buff + GO_SOUND_BUFFER_SIZE, 0);

			ProcessAudioSamplers(m_DetachedRelease[j], n_frames, this_buff);
		}

		double d = 1.0;
		d *= m_Volume;
		d *= 0.00000000059604644775390625;  // (2 ^ -24) / 100
		float f = d;
		std::fill(m_VolumeBuffer, m_VolumeBuffer + GO_SOUND_BUFFER_SIZE, f);

		for (unsigned int k = 0; k < n_frames*2; k++)
		{
			double d = this_buff[k];
			d *= m_VolumeBuffer[k];
			m_FinalBuffer[k] += d;
		}

	}

	m_CurrentTime += 1;

	/* Clamp the output */
	double clamp_min = -1.0, clamp_max = 1.0;
	if (meter_info)
	{
		unsigned used_samplers = m_SamplerPool.UsedSamplerCount();
		if (used_samplers > meter_info->current_polyphony)
			meter_info->current_polyphony = used_samplers;
		for (unsigned int k = 0; k < n_frames * 2; k += 2)
		{
			double d = std::min(std::max(m_FinalBuffer[k + 0], clamp_min), clamp_max);
			output_buffer[k + 0]  = (float)d;
			meter_info->meter_left = (meter_info->meter_left > output_buffer[k + 0]) ? meter_info->meter_left : output_buffer[k + 0];
			d = std::min(std::max(m_FinalBuffer[k + 1], clamp_min), clamp_max);
			output_buffer[k + 1]  = (float)d;
			meter_info->meter_right = (meter_info->meter_right > output_buffer[k + 1]) ? meter_info->meter_right : output_buffer[k + 1];
		}
	}
	else
	{
		for (unsigned int k = 0; k < n_frames * 2; k += 2)
		{
			double d = std::min(std::max(m_FinalBuffer[k + 0], clamp_min), clamp_max);
			output_buffer[k + 0]  = (float)d;
			d = std::min(std::max(m_FinalBuffer[k + 1], clamp_min), clamp_max);
			output_buffer[k + 1]  = (float)d;
		}
	}

	ResetDoneFlags();

	return 0;
}


SAMPLER_HANDLE GOSoundEngine::StartSample(const GOSoundProvider* pipe, int sampler_group_id)
{
	GO_SAMPLER* sampler = m_SamplerPool.GetSampler();
	if (sampler)
	{
		sampler->pipe = pipe;
		sampler->pipe_section = pipe->GetAttack();
		sampler->increment = sampler->pipe_section->sample_rate / (float) m_SampleRate;
		sampler->position = 0;
		memcpy
			(sampler->history
			,pipe->GetAttack()->history
			,sizeof(sampler->history)
			);
		//	else
		//	{
		//		sampler->stage = GSS_ATTACK;
		//		if (g_sound->HasRandomPipeSpeech() && !g_sound->windchests[m_WindchestGroup])
		//			sampler->position = rand() & 0x78;
		//	}
		sampler->fade = sampler->fademax = pipe->GetScaleAmplitude();
		sampler->shift = pipe->GetScaleShift();
		sampler->time = m_CurrentTime;
		StartSampler(sampler, sampler_group_id);
	}
	return sampler;
}

void GOSoundEngine::CreateReleaseSampler(const GO_SAMPLER* handle)
{
	if (!handle->pipe)
		return;

	const GOSoundProvider* this_pipe = handle->pipe;
	double vol = (handle->sampler_group_id < 0)
	           ? 1.0
	           : m_Windchests[handle->sampler_group_id - 1].windchest->GetVolume();

	// FIXME: this is wrong... the intention is to not create a release for a
	// sample being played back with zero amplitude but this is a comparison
	// against a double. We should test against a minimum level.
	if (vol)
	{
		GO_SAMPLER* new_sampler = m_SamplerPool.GetSampler();
		if (new_sampler != NULL)
		{

			const AUDIO_SECTION* release_section = this_pipe->GetRelease();
			new_sampler->pipe         = this_pipe;
			new_sampler->pipe_section = release_section;
			new_sampler->position     = 0;
			new_sampler->increment    = new_sampler->pipe_section->sample_rate / (float) m_SampleRate;
			new_sampler->shift        = this_pipe->GetScaleShift();
			new_sampler->time         = m_CurrentTime + 1;
			new_sampler->fademax      = this_pipe->GetScaleAmplitude();

			const bool not_a_tremulant = (handle->sampler_group_id >= 0);
			if (not_a_tremulant)
			{
				if (m_ScaledReleases)
				{
					int time = ((m_CurrentTime - handle->time) * (10 * BLOCKS_PER_FRAME)) / 441;
					if (time < 256)
						new_sampler->fademax = (this_pipe->GetScaleAmplitude() * (16384 + (time * 64))) >> 15;
					if (time < 1024)
						new_sampler->fadeout = 1; /* nominal = 1.5 seconds */
				}
				new_sampler->fademax = lrint(vol * new_sampler->fademax);
			}

			/* Determines how much fadein to apply every 2 samples. If the pipe
			 * has an amplitude of 10000 (which is nominal) and has been
			 * playing for a long period of time, this value will be equal to
			 *
			 *   ra_amp + 128 >> 8
			 * = -32640 >> 8
			 * = -128
			 *
			 * So for fade to reach fademax would take:
			 *
			 *   2 * fademax / fadein
			 * = 2 * -32768 / -128
			 * = 512 samples or
			 * = 12ms
			 */
			new_sampler->fadein = (new_sampler->fademax + 128) >> 8;
			if (new_sampler->fadein == 0)
				new_sampler->fadein--;

			/* This determines the period of time the release is allowed to
			 * fade in for in samples. 512 equates to roughly 12ms.
			 */
			new_sampler->faderemain = 512;

			/* FIXME: this must be enabled again at some point soon */
			if (m_ReleaseAlignmentEnabled && (release_section->release_aligner != NULL))
			{
				release_section->release_aligner->SetupRelease(*new_sampler, *handle);
			}
			else
			{
				new_sampler->position = 0; //m_release.offset;
				memcpy
					(new_sampler->history
					,release_section->history
					,sizeof(new_sampler->history)
					);
			}

			int windchest_index;
			if (not_a_tremulant)
			{
				/* detached releases are enabled and the pipe was on a regular
				 * windchest. Play the release on the detached windchest */
				int detached_windchest_index = 0;
				for(unsigned i = 1; i < m_DetachedRelease.size(); i++)
					if (m_DetachedRelease[i].count < m_DetachedRelease[detached_windchest_index].count)
						detached_windchest_index = i;
				if (detached_windchest_index)
					detached_windchest_index += m_Windchests.size();
				windchest_index = detached_windchest_index;
			}
			else
			{
				/* detached releases are disabled (or this isn't really a pipe)
				 * so put the release on the same windchest as the pipe (which
				 * means it will still be affected by tremulants - yuck). */
				windchest_index = handle->sampler_group_id;
			}
			if (handle->sampler_group_id == windchest_index)
				StartSamplerUnlocked(new_sampler, windchest_index);
			else
				StartSampler(new_sampler, windchest_index);
		}

	}
}


void GOSoundEngine::StopSample(const GOSoundProvider *pipe, SAMPLER_HANDLE handle)
{

	assert(handle);
	assert(pipe);

	// The following condition could arise if a one-shot sample is played,
	// decays away (and hence the sampler is discarded back into the pool), and
	// then the user releases a key. If the sampler had already been reused
	// with another pipe, that sample would erroneously be told to decay.
	if (pipe != handle->pipe)
		return;

	handle->stop = true;
}
