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

#include "GOrgueSoundCallbackAudio.h"

#include "GOrgueEnclosure.h"
#include "GOrguePipe.h"
#include "GOrgueSound.h"
#include "GOrgueWindchest.h"
#include "GrandOrgue.h"
#include "GrandOrgueFile.h"

extern GOrgueSound* g_sound;
extern GrandOrgueFile* organfile;

/* TODO: very grumpy that these are living here... */
void stereoUncompressed(GOrgueSampler* sampler,short* output)
{
    short* input=(short*)(sampler->ptr+sampler->current);
    output[0] = input[0];
    output[1] = input[1];
    output[2] = input[2];
    output[3] = input[3];
    sampler->current+=8;

}

void monoUncompressed(GOrgueSampler* sampler,short* output)
{
    short* input=(short*)(sampler->ptr+sampler->current);
    output[0] = input[0];
    output[1] = input[0];
    output[2] = input[1];
    output[3] = input[1];
    sampler->current+=4;

}

void monoCompressed(GOrgueSampler* sampler,short* output)
{
    short* v=(short*)&sampler->v;
    short* f=(short*)&sampler->f;
    wxByte* input=sampler->ptr+sampler->current;

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
        sampler->current+=4;

    }
    else
    {
        int inputAsInt1=*(int*)input;
        inputAsInt1>>=1;

        v[0]+=(((char*)(&inputAsInt1))[0]<<8)|((short)(input[2]));
        v[1]+=(((char*)(&inputAsInt1))[0]<<8)|((short)(input[2]));
        v[2]+=(((char*)(&inputAsInt1))[1]<<8)|((short)(input[3]));
        v[3]+=(((char*)(&inputAsInt1))[1]<<8)|((short)(input[3]));
        sampler->current+=8;
    }

    f[0]+=v[0];
    f[1]+=v[1];
    f[2]+=v[2];
    f[3]+=v[3];


    output[0] = f[0];
    output[1] = f[1];
    output[2] = f[2];
    output[3] = f[3];

}
//if the data is compressed, 32 bits represents 4 data ((Right and Left) * 2)
// we know the data is compressed because these 32bits ends with 1.
//if the data is uncompressed, 64 bits represents 4 data
// the first 32bits end with a 0.
void stereoCompressed(GOrgueSampler* sampler,short* output)
{
    short* v=(short*)&sampler->v;
    short* f=(short*)&sampler->f;
    wxByte* input=sampler->ptr+sampler->current;
//    printf("current %d input0 : %d v %d f %d\n",sampler->current, input[0],v[0],f[0]);

    // we are in little endian, so the byte the most to the right is in input[0]
    // check at the end of the first 32bits
    if(input[0]&0x01)
        // an int is 32 bit so we will use int
    {
        int inputAsInt=*(int*)input;
        inputAsInt>>=1;
        v[0]+=((char*)(&inputAsInt))[0];
        v[1]+=((char*)(&inputAsInt))[1];
        v[2]+=((char*)(&inputAsInt))[2];
        v[3]+=((char*)(&inputAsInt))[3];
        sampler->current+=4;

    }
    else
    {
        int inputAsInt1=*(int*)input;
        inputAsInt1>>=1;

        v[0]+=(((char*)(&inputAsInt1))[0]<<8)|((short)(input[4]));
        v[1]+=(((char*)(&inputAsInt1))[1]<<8)|((short)(input[5]));
        v[2]+=(((char*)(&inputAsInt1))[2]<<8)|((short)(input[6]));
        v[3]+=(((char*)(&inputAsInt1))[3]<<8)|((short)(input[7]));
        sampler->current+=8;
    }

    f[0]+=v[0];
    f[1]+=v[1];
    f[2]+=v[2];
    f[3]+=v[3];


    output[0] = f[0];
    output[1] = f[1];
    output[2] = f[2];
    output[3] = f[3];

}

// this callback will fill the buffer with bufferSize frame
// audio is opened with 32 bit stereo, so one frame is 64bit (32bit for right 32bit for left)
// So buffer can handle 8*bufferSize char (or 2*bufferSize float)
int GOrgueSound::AudioCallback(void *outputBuffer, void *inputBuffer,
                             unsigned int nFrames,
                             double streamTime,
                             RtAudioStreamStatus status,
                             void *userData )
{

	GOrgueSound* instance = (GOrgueSound*)userData;
	int i, j;
	float* buffer=static_cast<float*>(outputBuffer);

	if (organfile)
        organfile->SetElapsedTime(instance->sw.Time());

	/* Process any MIDI messages
	 * TODO: this is a very good candidate for a thread. */
	std::vector<unsigned char> msg;
	for (j = 0; j < instance->n_midiDevices; j++)
	{
		if (!instance->b_midiDevices[j])
			continue;

		for(;;)
		{
			instance->midiDevices[j]->getMessage(&msg);
			if (msg.empty())
				break;
			MIDICallback(msg, j, instance);
		}
	}

	/* if no samplers playing, or sound is disabled, fill buffer with zero
	 * and finish */
	if (!instance->b_active || !instance->samplers_count)
		memset(buffer, 0, nFrames);
	if (!instance->b_active)
		return 0;

	/* update the polyphony meter if a new peak has occured */
	if (instance->samplers_count > instance->meter_poly)
		instance->meter_poly = instance->samplers_count;

	/* initialise the output buffer */
	std::fill(instance->final_buff, instance->final_buff+2048,0);

	for (j = 0; j < organfile->GetTremulantCount() + 1 + organfile->GetWinchestGroupCount(); j++)
	{
		if (!instance->windchests[j])
			continue;

		GOrgueSampler* ptr = (GOrgueSampler*)(instance->windchests + j);

		int* this_buff;
		if (j < organfile->GetTremulantCount())
			this_buff = instance->g_buff[j + 1] + 2;
		else
		{
			this_buff = instance->g_buff[0] + 2;
			double d = 1.0, scale = 1.0/12700.0;
			for (i = 0; i < organfile->GetWindchest(j)->NumberOfEnclosures; i++)
			{
				GOrgueEnclosure* enclosure = organfile->GetEnclosure(organfile->GetWindchest(j)->enclosure[i]);
				d *= (double)(enclosure->MIDIValue * (100 - enclosure->AmpMinimumLevel) + 127 * enclosure->AmpMinimumLevel) * scale;
			}
			organfile->GetWindchest(j)->m_Volume = d;
			d *= instance->volume;
			d *= 0.00000000059604644775390625;  // (2 ^ -24) / 100

			float f = d;
			std::fill(instance->volume_buff, instance->volume_buff+2048,f);
		}

		std::fill(this_buff, this_buff + 2052, (j < organfile->GetTremulantCount() ? 0x800000 : 0));

		for (GOrgueSampler* sampler = instance->windchests[j]; sampler; sampler = sampler->next)
		{

			if (instance->b_limit && instance->samplers_count >= instance->poly_soft && sampler->stage == 2 && organfile->GetElapsedTime() - sampler->time > 250)
				sampler->fadeout = 4;

			int* write_iterator=this_buff;
//                printf("sampler %x fade %d fadein %d fadeout %d\n",sampler,sampler->fade,sampler->fadein,sampler->fadeout);
			for(unsigned int i=0;i<nFrames;i+=2)
			{
				short buffer[4];
				//Overflow should not be necessary
				if(sampler->type==2)
					stereoCompressed(sampler,buffer);
				if(sampler->type==3)
					stereoUncompressed(sampler,buffer);
				if(sampler->type==1)
					monoUncompressed(sampler,buffer);
				if(sampler->type==0)
					monoCompressed(sampler,buffer);

				if(sampler->stage != 2)
				{

					unsigned currentBlockLen = (sampler->stage == 1) ? sampler->pipe->m_loop.offset : sampler->pipe->m_attack.offset;

					if(sampler->current >= currentBlockLen)
					{
						sampler->stage = 1;
						sampler->ptr = sampler->pipe->m_loop.data;
						sampler->current = 0;//sampler->pipe->m_loop.offset;
						sampler->type = sampler->pipe->m_loop.type;
						sampler->f = *(wxInt64*)&sampler->pipe->m_loop.f;
						sampler->v = *(wxInt64*)&sampler->pipe->m_loop.v;
					}

				}
				else
				{
					if(sampler->current >= sampler->pipe->m_release.offset)
						sampler->pipe = NULL;
				}

				if(!sampler->pipe)
					break;

				int intbuffer[4];
				intbuffer[0]=buffer[0]*sampler->fade;
				intbuffer[1]=buffer[1]*sampler->fade;
				intbuffer[2]=buffer[2]*sampler->fade;
				intbuffer[3]=buffer[3]*sampler->fade;

				if(sampler->fadein+sampler->fadeout)
				{

					sampler->fade+=sampler->fadein+sampler->fadeout;
					if(sampler->fade>0)
						sampler->fade=0;
				}
				write_iterator[0]+=(intbuffer[0])>>sampler->shift;
				write_iterator[1]+=(intbuffer[1])>>sampler->shift;
				write_iterator[2]+=(intbuffer[2])>>sampler->shift;
				write_iterator[3]+=(intbuffer[3])>>sampler->shift;
//                    printf("write iter  %d  %d  %d %d\n", write_iterator[0],write_iterator[1],write_iterator[2],write_iterator[3]);
				write_iterator+=4;



			}
//                printf("fade %d fadein %d fadeout %d\n",sampler->fade,sampler->fadein,sampler->fadeout);

			if (!sampler->pipe && (!sampler->overflowing || !sampler->overflow))
				sampler->fade = 0;
			else if (sampler->fade < sampler->fademax)
				sampler->fadein = 0;
			else if (sampler->fadein)
			{
				sampler->faderemain -= nFrames;
				if (sampler->faderemain <= 0)
					sampler->fadein = 0;
			}

//            printf("sampler : %x sample_pipe:%x sampler_fade :%d current : %d\n",sampler,sampler->pipe,sampler->fade,sampler->current);
			if (!sampler->pipe || !sampler->fade)
			{
				ptr->next = sampler->next;
				instance->samplers_open[--(instance->samplers_count)] = sampler;
			}
			else
				ptr = sampler;
		}

		if (j >= organfile->GetTremulantCount())
		{
			int *ptr;
			for (i = 0; i < organfile->GetWindchest(j)->NumberOfTremulants; i++)
			{

				if (!instance->windchests[organfile->GetWindchest(j)->tremulant[i]])
					continue;
				ptr = instance->g_buff[organfile->GetWindchest(j)->tremulant[i] + 1] + 2;
				for (unsigned int k = 0; k < nFrames*2; k++)
				{
#ifdef linux
					//multiply by 2^-23
					instance->volume_buff[k] *= scalb(ptr[k], -23);
#endif
#ifdef _WIN32
					instance->volume_buff[k] *= _scalb(ptr[k], -23);
#endif
				}

			}

			ptr = this_buff;
			for (unsigned int k = 0; k < nFrames*2; k++)
			{
				double d = this_buff[k];
//                printf("data : %f\n",d);
				d *= instance->volume_buff[k];
				instance->final_buff[k] += d;
			}
		}
	}

	double clamp_min = -1.0, clamp_max = 1.0;
	for (unsigned int k = 0; k < nFrames*2; k += 2)
	{
		double d;
		d = std::min(std::max(instance->final_buff[k + 0], clamp_min) , clamp_max);
		((float*)buffer)[k + 0]  = (float)d;
		instance->meter_left  = std::max(instance->meter_left,fabs(d));
		d = std::min(std::max(instance->final_buff[k + 1] , clamp_min) , clamp_max);
		((float*)buffer)[k + 1]  = (float)d;
		instance->meter_right = std::max( instance->meter_right,fabs(d));
	}

	if (instance->f_output)
	{
		fwrite(buffer, sizeof(float), nFrames*2, instance->f_output);
		if (instance->b_stoprecording)
			instance->CloseWAV();
	}

	instance->meter_counter += nFrames;
	if (instance->meter_counter >= 6144)	// update 44100 / (N / 2) = ~14 times per second
	{
		int n = 0;
		// polyphony
		n |= ( 33 * instance->meter_poly ) / instance->polyphony;
		n <<= 8;
		// right channel
		n |= lrint(32.50000000000001 * instance->meter_right);
		n <<= 8;
		// left  channel
		n |= lrint(32.50000000000001 * instance->meter_left );

		wxCommandEvent event(wxEVT_METERS, 0);
		event.SetInt(n);
		::wxGetApp().frame->AddPendingEvent(event);

		instance->meter_counter = instance->meter_poly = 0;
		instance->meter_left = instance->meter_right = 0.0;
	}

	return 0;
}

