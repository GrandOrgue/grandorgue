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

#include "GOrguePipe.h"
#include "GOrgueSound.h"
#include "GOrgueTremulant.h"
#include "GOrgueWave.h"
#include "GOrgueWindchest.h"
#include "GrandOrgueFile.h"

extern GOrgueSound* g_sound;
extern GrandOrgueFile* organfile;

#define FREE_AND_NULL(x) do { if (x) { free(x); x = NULL; } } while (0)

/* FIXME: This function must be broken - not going to even start describing the problem.
 * TODO: "count" refers to the number of BYTES in the stream... not samples.
 *
 */

//void GrandOrgueFile::CompressWAV(char*& compress, short* fv, short* ptr, int count, int channels, int stage)
//{
//	int f[4] = {0, 0, 0, 0}, v[4] = {0, 0, 0, 0}, a[4] = {0, 0, 0, 0};
//	int size = 0, index;
//	char* origlength;
//	int i, j, count2;
//
//	/* not recommended, but if they want to, reduce stereo to mono
//	 * ^^ the above was an original comment.
//	 *
//	 * the stream is converted to mono and count still refers to the number of
//	 * bytes in the stream.
//	 */
//	if (channels == 2 && !(g_sound->IsStereo()))
//	{
//		for (int i = 0, j = 0; i < count; i += 2)
//			ptr[j++] = ((int)ptr[i] + (int)ptr[i + 1]) >> 1;
//		count >>= 1;
//		channels = 1;
//	}
//
//	/* maxsearch is the number of BYTES to look for phase alignment
//	 */
//	int maxsearch = 2206 * channels;	// ~20Hz maximum search width for phase alignment table
//	if (maxsearch > count)
//		maxsearch = count - (count & channels);
//
//	origlength = compress;
//	m_compress_p->types[stage] = (channels - 1) << 1;
//
//	/*
//	 * Initialise f's and v's in preparation for compression/phase alignment
//	 */
//	for (i = 0; i < 4; i++)
//	{
//		// stereo : j = i
//		// mono   : j = i / 2
//		j = i >> (2 - channels);
//		if (count > channels * 2)
//            v[i] = fv[i + 12] = ptr[j + channels * 2] - ptr[j];
//		else
//			v[i] = fv[i + 12] = 0;
//		f[i] = fv[i] = ptr[j] - v[i];
//	}
//
//	bool flags[PHASE_ALIGN_RES_VA];
//	if (stage == 2)
//	{
//		for (j = 0; j < PHASE_ALIGN_RES_VA; j++)
//		{
//			flags[j] = false;
//			m_compress_p->ra_offset[j] = 0;
//			for (i = 0; i < 4; i++)
//			{
//				m_compress_p->ra_f[j][i] = f[i];
//				m_compress_p->ra_v[j][i] = v[i];
//			}
//		}
//
//		int prev_index = m_compress_p->ra_getindex( f, v), prev_i = 0;
//		for (i = 0, j = 0; i < maxsearch; )
//		{
//			v[j] = ptr[i] - f[j];
//			f[j] = ptr[i];
//			j++;
//			if (channels == 1)
//			{
//				v[j] = v[j-1];
//				f[j] = f[j-1];
//				j++;
//			}
//			j &= 3;
//			i++;
//			if (!j)
//			{
//				index = m_compress_p->ra_getindex( f, v);
//				if(index>=PHASE_ALIGN_RES_VA)
//				  std::cout << index << ":index out of bound\n";
//				if (index != prev_index)
//				{
//					if (!m_compress_p->ra_offset[prev_index])
//						m_compress_p->ra_offset[prev_index] = ((prev_i + i) >> (channels + 1)) << (channels + 1);
//					prev_i = i;
//					prev_index = index;
//				}
//			}
//		}
//
//		for (i = 0; i < 4; i++)
//		{
//			f[i] = fv[i];
//			v[i] = fv[i + 12];
//		}
//	}
//
//	if (!m_b_squash)
//		goto CantCompress;
//
//	count2 = count + (count & channels);
//	if (count & channels)
//		for (j = 0; j < channels; j++)
//			*(ptr + count + j) = 2 * (int)*(ptr + count + j - 2 * channels) - (int)*(ptr + count + j - 4 * channels);
//
//	if (channels == 2)
//	{
//		for (i = 0; i < count2; )
//		{
//			int value;
//			value = ptr[i] - f[0];
//			a[0] = value - v[0];
//			v[0] = value;
//			f[0] = ptr[i++];
//			value = ptr[i] - f[1];
//			a[1] = value - v[1];
//			v[1] = value;
//			f[1] = ptr[i++];
//			value = ptr[i] - f[2];
//			a[2] = value - v[2];
//			v[2] = value;
//			f[2] = ptr[i++];
//			value = ptr[i] - f[3];
//			a[3] = value - v[3];
//			v[3] = value;
//			f[3] = ptr[i++];
//
//            if (((((v[0] + 32768) | (v[1] + 32768) | (v[2] + 32768) | (v[3] + 32768)) >> 16) | (((a[0] + 16384) | (a[1] + 16384) | (a[2] + 16384) | (a[3] + 16384)) >> 15)) && i < count)
//                goto CantCompress;
//            if ( (((a[0] + 128) | (a[1] + 128) | (a[2] + 128)) >> 8) | ((a[3] + 64) >> 7))
//            {
//                *(int*)(compress     ) = ((a[3] & 0xFF00) << 17) | ((a[2] & 0xFF00) <<  9) | ((a[1] & 0xFF00) <<  1) | ((a[0] & 0xFF00) >>  7);
//                *(int*)(compress += 4) = ((a[3] & 0x00FF) << 24) | ((a[2] & 0x00FF) << 16) | ((a[1] & 0x00FF) <<  8) | ((a[0] & 0x00FF)      );
//                size += i <= count ? 8 : 4;
//            }
//            else
//            {
//                *(int*)(compress     ) = ((a[3] & 0x00FF) << 25) | ((a[2] & 0x00FF) << 17) | ((a[1] & 0x00FF) <<  9) | ((a[0] & 0x00FF) <<  1) | 1;
//                size += i <= count ? 4 : 2;
//            }
//            compress += 4;
//
//            if (i < maxsearch && stage == 2)
//            {
//                index = m_compress_p->ra_getindex( f, v);
//                if (!flags[index] && m_compress_p->ra_offset[index] == i << 1)
//                {
//                    flags[index] = true;
//                    m_compress_p->ra_offset[index] = size;
//                    for (j = 0; j < 4; j++)
//                    {
//                        m_compress_p->ra_f[index][j] = f[j];
//                        m_compress_p->ra_v[index][j] = v[j];
//                    }
//                }
//            }
//		}
//	}
//	else
//	{
//		for (i = 0; i < count2; )
//		{
//			int value;
//			value = ptr[i] - f[0];
//			a[0] = value - v[0];
//			v[0] = value;
//			f[0] = ptr[i++];
//			value = ptr[i] - f[2];
//			a[2] = value - v[2];
//			v[2] = value;
//			f[2] = ptr[i++];
//
//            if (((((v[0] + 32768) | (v[2] + 32768)) >> 16) | (((a[0] + 16384) | (a[2] + 16384)) >> 15)) && i < count)
//                goto CantCompress;
//
//            if (((a[0] + 128) >> 8) | ((a[2] + 64) >> 7))
//            {
//                *(short*)(compress     ) = ((a[2] & 0xFF00) <<  1) | ((a[0] & 0xFF00) >>  7);
//                *(short*)(compress += 2) = ((a[2] & 0x00FF) <<  8) | ((a[0] & 0x00FF)      );
//                size += i <= count ? 4 : 2;
//            }
//            else
//            {
//                *(short*)(compress     ) = ((a[2] & 0x00FF) <<  9) | ((a[0] & 0x00FF) <<  1) | 1;
//                size += i <= count ? 2 : 1;
//            }
//            compress += 2;
//
//            if (i < maxsearch && stage == 2)
//            {
//                f[1] = f[0];
//                f[3] = f[2];
//                v[1] = v[0];
//                v[3] = v[2];
//                index = m_compress_p->ra_getindex( f, v);
//                if (!flags[index] && m_compress_p->ra_offset[index] == i << 1)
//                {
//                    flags[index] = true;
//                    m_compress_p->ra_offset[index] = size;
//                    for (j = 0; j < 4; j++)
//                    {
//                        m_compress_p->ra_f[index][j] = f[j];
//                        m_compress_p->ra_v[index][j] = v[j];
//                    }
//                }
//            }
//		}
//	}
//	compress += channels << 1;
//
//	goto Done;
//
//CantCompress:
//	m_compress_p->types[stage] |= 1;
//
//	compress = origlength;
//	memcpy(compress, ptr, sizeof(short) * count);
//	memcpy(compress+(sizeof(short) * count),ptr, sizeof(short) * (count & channels));
//	compress=compress+(sizeof(short)*((count&channels) + count));
//
//Done:
//	m_compress_p->ptr[stage]    = (wxByte*)((wxByte*)(origlength + size) - (wxByte*)m_compress_p);
//	m_compress_p->offset[stage] = -size;
//	if (stage == 2)
//		for (j = 0; j < PHASE_ALIGN_RES_VA; j++)
//			m_compress_p->ra_offset[j] -= size;
//}

GOrguePipe::~GOrguePipe()
{
	FREE_AND_NULL(m_attack.data);
	FREE_AND_NULL(m_loop.data);
	FREE_AND_NULL(m_release.data);
}

GOrguePipe::GOrguePipe()
{
	memset(&m_attack, 0, sizeof(m_attack));
	memset(&m_loop, 0, sizeof(m_loop));
	memset(&m_release, 0, sizeof(m_release));

	_fourcc = 0;
	_adler32 = 0;
	_this = NULL;
	pitch = 0;
	sampler = NULL;
	instances = 0;
	WindchestGroup = 0;
	ra_amp = 0;
	ra_shift = 0;

}

void GOrguePipe::SetOn()
{

	if (instances > 0)
	{
		instances++;
		return;
	}

	GOrgueSampler* sampler = g_sound->OpenNewSampler();
	if (sampler == NULL)
		return;

	if (instances < 0)
	{
		sampler->stage = 2;
	}
	else
	{
		sampler->stage = 0;
		if (g_sound->HasRandomPipeSpeech() && !g_sound->windchests[WindchestGroup])
			sampler->offset = rand() & 0x78;
	}

	sampler->type = m_attack.type;
	sampler->fade = sampler->fademax = ra_amp;

	sampler->current = 0;// m_attack.offset;
	sampler->ptr = m_attack.data;
	sampler->pipe = this;
	sampler->shift = ra_shift;
	sampler->f = *(wxInt64*)(m_attack.f); // FIXME: WEIRD!
	sampler->v = *(wxInt64*)(m_attack.v);
	sampler->time = organfile->GetElapsedTime();

	this->sampler = sampler;

	sampler->next = g_sound->windchests[WindchestGroup];
	g_sound->windchests[WindchestGroup] = sampler;

	if (instances == 0)
		instances++;

}

void GOrguePipe::SetOff()
{
	if (instances <= 0)
		return;

	instances--;

	if (instances > 0)
		return;

	double vol = organfile->GetWindchest(WindchestGroup)->GetVolume();
	if (vol)
	{
		GOrgueSampler* sampler = g_sound->OpenNewSampler();
		if (sampler != NULL)
		{
			sampler->current = 0;//m_release.offset;
			sampler->ptr = m_release.data;
			sampler->pipe = this;
			sampler->shift = ra_shift;
			sampler->stage = 2;
			sampler->type = m_release.type;
			int time = organfile->GetElapsedTime() - this->sampler->time;
			sampler->time = organfile->GetElapsedTime();
			sampler->fademax = ra_amp;
			if (g_sound->HasScaledReleases() && WindchestGroup >= organfile->GetTremulantCount())
			{
				if (time < 256)
					sampler->fademax = (sampler->fademax * (16384 + (time << 5))) >> 15;
				if (time < 1024)
					sampler->fadeout = 0x0001;
			}
			if (g_sound->b_detach && WindchestGroup >= organfile->GetTremulantCount())
				sampler->fademax = lrint(((double)sampler->fademax) * vol);
			sampler->fadein = (sampler->fademax + 128) >> 8;
			if (!sampler->fadein)
				sampler->fadein--;
			sampler->faderemain = 512;	// 32768*65536 / 64*65536

#if 0
			/* FIXME: this must be enabled again at some point soon */
			if (g_sound->HasReleaseAlignment())
			{
				int index = ra_getindex((short*)&this->sampler->f, (short*)&this->sampler->v);
				sampler->current = ra_offset[index];
				sampler->f = *(wxInt64*)(ra_f[index]);
				sampler->v = *(wxInt64*)(ra_v[index]);
			}
			else
#endif
			{
				sampler->current = 0; //m_release.offset;
				sampler->f = *(wxInt64*)(m_release.f);
				sampler->v = *(wxInt64*)(m_release.v);
			}

			if (g_sound->b_detach && WindchestGroup >= organfile->GetTremulantCount())
			{
				sampler->next = g_sound->windchests[organfile->GetTremulantCount()];
				g_sound->windchests[organfile->GetTremulantCount()] = sampler;
			}
			else
			{
				sampler->next = g_sound->windchests[WindchestGroup];
				g_sound->windchests[WindchestGroup] = sampler;
			}
		}
	}
    sampler->fadeout = (-ra_amp - 128) >> 8;
    if (!sampler->fadeout)
        sampler->fadeout++;
	this->sampler = 0;

}

void GOrguePipe::Set(bool on)
{
	if (on)
		SetOn();
	else
		SetOff();
}

void GOrguePipe::LoadFromFile(const wxString& filename, int amp)
{

	GOrgueWave wave;
	wave.Open(filename);

	/* allocate data to work with */
	wxInt16* data = (wxInt16*)malloc(wave.GetChannels() * wave.GetLength() * sizeof(wxInt16));
	if (data == NULL)
		throw (char*)"< out of memory allocating wave";

	try
	{

		wave.ReadSamples(data, GOrgueWave::SF_SIGNEDSHORT, 44100);

		/* FIXME: These were related to releases... probably need investigation*/
		// m_compress_p->ra_volume = wave.GetPeak();
		// m_compress_p->ra_factor = (PHASE_ALIGN_RES << 25) / (wave.GetPeak() + 1);

		/* FIXME: This code was present in the original version (however it
		 * has been modified extremely heavily).
		 *
		 * Basically, sample playback reads blocks of two samples at a time,
		 * which means that if the loop ranges, attack segment length or
		 * release segment length is not a multiple of 2, the sound will
		 * crack. This is a hack to restore playback to something which should
		 * behave - but there are likely to be situations where it won't and
		 * I believe that these are the situations that have been reported on
		 * the bug tracker related to clicks in the audio.
		 */

		unsigned attackSamples = wave.GetLength();

		if (wave.HasLoops() && wave.HasReleaseMarker())
		{

			attackSamples = wave.GetLoopStartPosition();
			unsigned loopStart = wave.GetLoopStartPosition();
			assert(loopStart > 0);
			assert(wave.GetLoopEndPosition() > loopStart);
			unsigned loopSamples = wave.GetLoopEndPosition() - loopStart + 1;
			unsigned releaseOffset = wave.GetReleaseMarkerPosition();
			unsigned releaseSamples = wave.GetLength() - releaseOffset;

			bool loopSampleAdded = false;
			bool releaseSampleAdded = false;

			if (attackSamples % 2)
			{
				attackSamples -= 1;
				loopStart -= 1;
			}

			if (loopSamples % 2)
			{
				loopSamples += 1;
				loopSampleAdded = true;
			}

			if (releaseSamples % 2)
			{
				releaseSamples += 1;
				releaseSampleAdded = true;
			}

			m_loop.offset = (int)loopSamples * sizeof(wxInt16) * wave.GetChannels();
			m_loop.data = (unsigned char*)malloc(wave.GetChannels() * loopSamples * sizeof(wxInt16));
			if (m_loop.data == NULL)
				throw (char*)"< out of memory allocating loop";

			memcpy(m_loop.data,
				&data[loopStart * wave.GetChannels()],
				m_loop.offset);

			if (loopSampleAdded)
			{
				for (unsigned int i = 0; i < wave.GetChannels(); i++)
				{
					wxInt16 a = *(wxInt16*)&m_loop.data[i * sizeof(wxInt16)];
					wxInt16 *b = (wxInt16*)&m_loop.data[((loopSamples - 2) * wave.GetChannels() + i) * sizeof(wxInt16)];
					a = (*b + a) / 2;
					b += wave.GetChannels();
					*b = a;
				}
			}

			assert(loopStart < attackSamples + 1);

			m_release.offset = (int)releaseSamples * sizeof(wxInt16) * wave.GetChannels();
			m_release.data = (unsigned char*)malloc(m_release.offset);
			if (m_release.data == NULL)
				throw (char*)"< out of memory allocating release";

			memcpy(m_release.data,
				&data[(wave.GetLength() - releaseSamples - 1) * wave.GetChannels()],
				m_release.offset);

		}

		/* Load attack segment */
		assert(attackSamples != 0);
		m_attack.offset = (int)attackSamples * sizeof(wxInt16) * wave.GetChannels();
		m_attack.data = (unsigned char*)malloc(m_attack.offset);
		if (m_attack.data == NULL)
			throw (char*)"< out of memory allocating attack";

		memcpy(m_attack.data,
			&data[0],
			m_attack.offset);

		ra_shift = 7;
		while (amp > 10000)
		{
			ra_shift--;
			amp >>= 1;
		}

		ra_amp = (amp << 15) / -10000;
		sampler = NULL;

		if (wave.GetChannels() == 1)
		{
			m_attack.type = 1;
			m_loop.type = 1;
			m_release.type = 1;
		}
		else
		{
			m_attack.type = 3;
			m_loop.type = 3;
			m_release.type = 3;
		}
		free(data);

	}
	catch (char* error)
	{
		fprintf(stderr, "caught exception: %s\n", error);
		free(data);
		FREE_AND_NULL(m_attack.data);
		FREE_AND_NULL(m_loop.data);
		FREE_AND_NULL(m_release.data);
		throw;
	}
	catch (...)
	{
		free(data);
		FREE_AND_NULL(m_attack.data);
		FREE_AND_NULL(m_loop.data);
		FREE_AND_NULL(m_release.data);
		throw;
	}

}



/*
int GOrguePipe::ra_getindex(int *f, int *v)
{
	int retval = 0;
	if (v[0] + v[1] + v[2] + v[3] >= 0)
		retval = PHASE_ALIGN_RES;
	retval += (((((f[0] + f[1] + f[2] + f[3]) >> 2) + ra_volume) * ra_factor) >> 26);
	wxASSERT(retval >= 0 && retval < PHASE_ALIGN_RES_VA);
	return retval;
}

int GOrguePipe::ra_getindex(short *f, short *v)
{
	int retval = 0;
	if ((int)v[0] + (int)v[1] + (int)v[2] + (int)v[3] >= 0)
		retval = PHASE_ALIGN_RES;
	retval += ((((((int)f[0] + (int)f[1] + (int)f[2] + (int)f[3]) >> 2) + ra_volume) * ra_factor) >> 26);
	wxASSERT(retval >= 0 && retval < PHASE_ALIGN_RES_VA);
	return retval;
}
*/

short SynthTrem(double amp, double angle)
{
	return (short)(amp * sin(angle));
}

short SynthTrem(double amp, double angle, double fade)
{
	return (short)(fade * amp * sin(angle));
}

void GOrguePipe::CreateFromTremulant(GOrgueTremulant* tremulant)
{

	int amp = 10000;
	int peak = 16384 * tremulant->AmpModDepth / 100;
	double trem_amp = (16384 * tremulant->AmpModDepth / 100);
	double trem_param = 0.14247585730565955729989312395825 / (double)tremulant->Period;

	unsigned loopstart = 441000 / tremulant->StartRate;
	unsigned loopend = loopstart + 441 * tremulant->Period / 10;
	unsigned release = loopend;
	unsigned wavestart = 0;
	unsigned wavesize = release + 441000 / tremulant->StopRate;
	unsigned length = sizeof(short) * wavesize;

	try
	{

		unsigned q, r;
		int  k, jj;

		double trem_angle = 0.0;

		/* Generate startup */
		m_attack.data = (unsigned char*)malloc(loopstart * sizeof(wxInt16));
		if (m_attack.data == NULL)
			throw (char*)"< out of memory";
		for (unsigned j = 0; j < loopstart; j++)
		{
			double trem_fade = (double)j / loopstart;
			((wxInt16*)m_attack.data)[j] = SynthTrem(trem_amp, trem_angle, trem_fade);
			trem_angle += trem_param * trem_fade;
		}

		/* Generate loop */
		m_loop.data = (unsigned char*)malloc((loopend - loopstart + 1) * sizeof(wxInt16));
		if (m_loop.data == NULL)
			throw (char*)"< out of memory";
		for (unsigned j = 0; j < (loopend - loopstart + 1); j++)
		{
			((wxInt16*)m_loop.data)[j] = SynthTrem(trem_amp, trem_angle);
			trem_angle += trem_param;
		}

		/* Generate release */
		m_release.data = (unsigned char*)malloc((wavesize - loopend - 1) * sizeof(wxInt16));
		if (m_release.data == NULL)
			throw (char*)"< out of memory";
		for (unsigned j = 0; j < (wavesize - loopend - 1); j++)
		{
			double trem_fade = (double)(j - loopend - 1) / (wavesize - loopend - 1);
			((wxInt16*)m_release.data)[j] = SynthTrem(trem_amp, trem_angle, trem_fade);
			trem_angle += trem_param * trem_fade;
		}

		ra_shift = 7;
		while (amp > 10000)
		{
			ra_shift--;
			amp >>= 1;
		}
		ra_amp = (amp << 15) / -10000;
		sampler = NULL;
		m_attack.type = 1;
		m_loop.type = 1;
		m_release.type = 1;

	}
	catch (...)
	{
		FREE_AND_NULL(m_attack.data);
		FREE_AND_NULL(m_loop.data);
		FREE_AND_NULL(m_release.data);
		throw;
	}

}
