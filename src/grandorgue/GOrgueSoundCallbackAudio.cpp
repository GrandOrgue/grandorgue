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

#include "GOrgueSound.h"

#include "GOrgueEnclosure.h"
#include "GOrgueMidi.h"
#include "GOrguePipe.h"
#include "GOrgueReleaseAlignTable.h"
#include "GOrgueWindchest.h"
#include "GrandOrgue.h"
#include "GrandOrgueFile.h"
#include "RtAudio.h"

static
inline
void stereoUncompressed
	(GO_SAMPLER* sampler
	,int* output
	)
{

	// "borrow" the output buffer to compute release alignment info
	wxInt16* input = (wxInt16*)(sampler->pipe_section->data + sampler->position);

	// copy the sample buffer
	for (unsigned int i = 0; i < BLOCKS_PER_FRAME * 2; input++, output++, i++)
		*output = *input;

	// update sample history (for release alignment / compression)
	input -= BLOCK_HISTORY * 2;
	for (unsigned i = 0; i < BLOCK_HISTORY; i++)
	{
		sampler->history[i * MAX_OUTPUT_CHANNELS + 0] = input[i * 2 + 0];
		sampler->history[i * MAX_OUTPUT_CHANNELS + 1] = input[i * 2 + 1];
	}

	// update the position
	sampler->position += BLOCKS_PER_FRAME * sizeof(wxInt16) * 2;

}

static
inline
void monoUncompressed
	(GO_SAMPLER* sampler
	,int* output
	)
{

	// copy the sample buffer
	wxInt16* input = (wxInt16*)(sampler->pipe_section->data + sampler->position);
	for (unsigned int i = 0; i < BLOCKS_PER_FRAME; i++, input++, output += 2)
	{
		output[0] = *input;
		output[1] = *input;
	}

	// update sample history (for release alignment / compression)
	input -= BLOCK_HISTORY;
	for (unsigned i = 0; i < BLOCK_HISTORY; i++, input++)
		sampler->history[i * MAX_OUTPUT_CHANNELS] = *input;

	// update the position
	sampler->position += BLOCKS_PER_FRAME * sizeof(wxInt16);

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

static
inline
void ReadSamplerFrames
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
			if (sampler->position >= sampler->pipe->GetRelease()->size)
				sampler->pipe = NULL;

		}
		else
		{

			unsigned currentBlockSize = sampler->pipe_section->size;
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
				}
			}
		}

	}

}

inline
void GOrgueSound::ProcessAudioSamplers
	(GO_SAMPLER** list_start
	,unsigned int n_frames
	,int* output_buffer
	)
{

	assert(list_start);
	assert((n_frames & (n_frames - 1)) == 0);
	assert(n_frames > BLOCKS_PER_FRAME);
	GO_SAMPLER* previous_valid_sampler = *list_start;
	for (GO_SAMPLER* sampler = *list_start; sampler; sampler = sampler->next)
	{

		if  (
				(b_limit) &&
				(samplers_count >= poly_soft) &&
				(sampler->pipe_section->stage == GSS_RELEASE) &&
				(m_organfile->GetElapsedTime() - sampler->time > 250)
			)
			sampler->fadeout = 4;

		ReadSamplerFrames
			(sampler
			,n_frames
			,m_TempDecodeBuffer
			);

		ApplySamplerFade
			(sampler
			,n_frames
			,m_TempDecodeBuffer
			);

		/* Add these samples to the current output buffer shifting
		 * right by the necessary amount to bring the sample gain back
		 * to unity (this value is computed in GOrguePipe.cpp)
		 */
		int shift = sampler->shift;
		int* write_iterator = output_buffer;
		int* decode_pos = m_TempDecodeBuffer;
		for(unsigned int i = 0; i < n_frames / 2; i++, write_iterator += 4, decode_pos += 4)
		{
			write_iterator[0] += decode_pos[0] >> shift;
			write_iterator[1] += decode_pos[1] >> shift;
			write_iterator[2] += decode_pos[2] >> shift;
			write_iterator[3] += decode_pos[3] >> shift;
		}

		/* if this sampler's pipe has been set to null or the fade value is
		 * zero, the sample is no longer required and can be removed from the
		 * linked list. If it was still supplying audio, we must update the
		 * previous valid sampler. */
		// printf("sampler : %x sample_pipe:%x sampler_fade :%d current : %d\n",sampler,sampler->pipe,sampler->fade,sampler->current);
		if (!sampler->pipe || !sampler->fade)
		{
			/* sampler needs to be removed from the list */
			if (sampler == *list_start)
				*list_start = sampler->next;
			previous_valid_sampler->next = sampler->next;
			samplers_open[--samplers_count] = sampler;
		}
		else
		{
			previous_valid_sampler = sampler;
		}

	}

}

// this callback will fill the buffer with bufferSize frame
// audio is opened with 32 bit stereo, so one frame is 64bit (32bit for right 32bit for left)
// So buffer can handle 8*bufferSize char (or 2*bufferSize float)
inline
int GOrgueSound::AudioCallbackLocal
	(float *output_buffer
	,unsigned int n_frames
	,double stream_time
	)
{

	if (m_organfile)
		m_organfile->SetElapsedTime(sw.Time());

	m_midi->ProcessMessages(b_active);

	/* if no samplers playing, or sound is disabled, fill buffer with zero */
	if (!b_active || !samplers_count || !m_organfile)
	{

		memset(output_buffer, 0, (n_frames * sizeof(float)));

		/* we can only abort for the case of the sound system not being active
		 * (because recording could be enabled... */
		if (!b_active || !m_organfile)
			return 0;

	}

	/* update the polyphony meter if a new peak has occured */
	if (samplers_count > meter_poly)
		meter_poly = samplers_count;

	/* initialise the output buffer */
	std::fill(final_buff, final_buff + 2048, 0);

	for (unsigned j = 0; j < m_organfile->GetTremulantCount(); j++)
	{
		if (m_tremulants[j].sampler == NULL)
			continue;

		int* this_buff = m_tremulants[j].buff;

		std::fill (this_buff, this_buff + 2048, 0x800000);

		ProcessAudioSamplers (&(m_tremulants[j].sampler), n_frames, this_buff);
	}

	if (m_detachedRelease != NULL)
	{
		int* this_buff = g_buff;

		std::fill (this_buff, this_buff + 2048, 0);

		ProcessAudioSamplers(&m_detachedRelease, n_frames, this_buff);

		double d = 1.0;
		d *= volume;
		d *= 0.00000000059604644775390625;  // (2 ^ -24) / 100
		float f = d;
		std::fill(volume_buff, volume_buff + 2048, f);

		for (unsigned int k = 0; k < n_frames*2; k++)
		{
			double d = this_buff[k];
			d *= volume_buff[k];
			final_buff[k] += d;
		}

	}

	for (unsigned j = 0; j < m_organfile->GetWinchestGroupCount(); j++)
	{

		if (m_windchests[j] == NULL)
			continue;

		int* this_buff = g_buff;

		std::fill (this_buff, this_buff + 2048, 0);

		ProcessAudioSamplers(&(m_windchests[j]), n_frames, this_buff);

		GOrgueWindchest* current_windchest = m_organfile->GetWindchest(j);
		double d = current_windchest->GetVolume();
		d *= volume;
		d *= 0.00000000059604644775390625;  // (2 ^ -24) / 100
		float f = d;
		std::fill(volume_buff, volume_buff + 2048, f);

		for (unsigned i = 0; i < current_windchest->GetTremulantCount(); i++)
		{
			unsigned tremulant_pos = current_windchest->GetTremulantId(i);
			if (!m_tremulants[tremulant_pos].sampler)
				continue;
			int *ptr = m_tremulants[tremulant_pos].buff;
			for (unsigned int k = 0; k < n_frames*2; k++)
			{
				//multiply by 2^-23
				volume_buff[k] *= ldexp(ptr[k], -23);
			}
		}

		for (unsigned int k = 0; k < n_frames*2; k++)
		{
			double d = this_buff[k];
			d *= volume_buff[k];
			final_buff[k] += d;
		}

	}

	/* Clamp the output */
	double clamp_min = -1.0, clamp_max = 1.0;
	for (unsigned int k = 0; k < n_frames * 2; k += 2)
	{

		double d = std::min(std::max(final_buff[k + 0], clamp_min), clamp_max);
		output_buffer[k + 0]  = (float)d;
		meter_left  = std::max(meter_left, fabs(d));

		d = std::min(std::max(final_buff[k + 1], clamp_min), clamp_max);
		output_buffer[k + 1]  = (float)d;
		meter_right = std::max(meter_right, fabs(d));

	}

	/* Write data to file if recording is enabled*/
	if (f_output)
	{
		fwrite(output_buffer, sizeof(float), n_frames * 2, f_output);
		if (b_stoprecording)
			CloseWAV();
	}

	/* Update the meter */
	meter_counter += n_frames;
	if (meter_counter >= 6144)	// update 44100 / (N / 2) = ~14 times per second
	{

		// polyphony
		int n = ( 33 * meter_poly ) / polyphony;
		n <<= 8;
		// right channel
		n |= lrint(32.50000000000001 * meter_right);
		n <<= 8;
		// left  channel
		n |= lrint(32.50000000000001 * meter_left );

		wxCommandEvent event(wxEVT_METERS, 0);
		event.SetInt(n);
		::wxGetApp().frame->AddPendingEvent(event);

		meter_counter = meter_poly = 0;
		meter_left = meter_right = 0.0;

	}

	return 0;
}

int
GOrgueSound::AudioCallback
	(void *outputBuffer
	,void *inputBuffer
	,unsigned int nFrames
	,double streamTime
	,RtAudioStreamStatus status
	,void *userData)
{

	assert(userData);

	if (nFrames & (BLOCKS_PER_FRAME - 1))
	{
		wxString error;
		error.Printf
			(_("Audio callback of %u blocks requested. Must be divisible by %u.")
			,nFrames
			,BLOCKS_PER_FRAME
			);
		throw error;
	}

	return ((GOrgueSound*)userData)->AudioCallbackLocal
		(static_cast<float*>(outputBuffer)
		,nFrames
		,streamTime
		);

}
