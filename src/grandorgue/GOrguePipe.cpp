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
#include "GOrgueStop.h"
#include "GOrgueTremulant.h"
#include "GOrgueWave.h"
#include "GOrgueWindchest.h"
#include "GOrgueReleaseAlignTable.h"
#include "GrandOrgueFile.h"

extern GOrgueSound* g_sound;
extern GrandOrgueFile* organfile;

/* This parameter defines how many samples should be added to the end of each
 * audio section (i.e. the "attack", "loop" or "release" segment). It is
 * necessary due to how the sampler fetches samples. Probably not a good idea
 * to lower this value unless you know exactly what you're doing.
 */
#define SAMPLE_SLACK 2

#define FREE_AND_NULL(x) do { if (x) { free(x); x = NULL; } } while (0)

void GOrguePipe::GetMaxAmplitudeAndDerivative
	(AUDIO_SECTION& section
	,int& runningMaxAmplitude
	,int& runningMaxDerivative
	)
{

	assert((section.size % (m_Channels * sizeof(wxInt16))) == 0);
	unsigned int sectionLen = section.size / (m_Channels * sizeof(wxInt16));

	int f = 0; /* to avoid compiler warning */
	for (unsigned int i = 0; i < sectionLen; i++)
	{

		/* Get sum of amplitudes in channels */
		int f_p = f;
		f = 0;
		for (unsigned int j = 0; j < m_Channels; j++)
			f += *((wxInt16*)&section.data[(i * m_Channels + j) * sizeof(wxInt16)]);

		if (abs(f) > runningMaxAmplitude)
			runningMaxAmplitude = abs(f);

		if (i == 0)
			continue;

		/* Get v */
		int v = f - f_p;
		if (abs(v) > runningMaxDerivative)
			runningMaxDerivative = abs(v);

	}

}

/* REGRESSION TODO: It would be good for somebody to do some rigorous
 * testing on this code before this comment is removed. It is designed
 * based on a dream. */
void GOrguePipe::ComputeReleaseAlignmentInfo()
{

	FREE_AND_NULL(m_ra_table);

	/* Find the maximum amplitude and derivative of the waveform */
	int phase_align_max_amplitude = 0;
	int phase_align_max_derivative = 0;
	GetMaxAmplitudeAndDerivative(m_attack,  phase_align_max_amplitude, phase_align_max_derivative);
	GetMaxAmplitudeAndDerivative(m_loop,    phase_align_max_amplitude, phase_align_max_derivative);
	GetMaxAmplitudeAndDerivative(m_release, phase_align_max_amplitude, phase_align_max_derivative);

	if ((phase_align_max_derivative != 0) && (phase_align_max_amplitude != 0))
	{

		m_ra_table = new GOrgueReleaseAlignTable();
		m_ra_table->ComputeTable
			(m_release
			,phase_align_max_amplitude
			,phase_align_max_derivative
			,m_SampleRate
			,m_Channels
			);

	}

}

GOrguePipe::~GOrguePipe()
{
	FREE_AND_NULL(m_attack.data);
	FREE_AND_NULL(m_loop.data);
	FREE_AND_NULL(m_release.data);
	FREE_AND_NULL(m_ra_table);
}

GOrguePipe::GOrguePipe(wxString filename, bool percussive, int windchestGroup, int amplitude)
{
	m_filename = filename;
	m_percussive = percussive;
	m_WindchestGroup = windchestGroup;
	m_amplitude = amplitude;
	m_ref = NULL;

	memset(&m_attack, 0, sizeof(m_attack));
	memset(&m_loop, 0, sizeof(m_loop));
	memset(&m_release, 0, sizeof(m_release));

	m_ra_table = NULL;
	pitch = 0;
	sampler = NULL;
	instances = 0;
	ra_amp = 0;
	ra_shift = 0;
	m_SampleRate = 0;
	m_Channels = 0;
}

void GOrguePipe::SetOn()
{

	if (instances > 0)
	{
		instances++;
		return;
	}

	GO_SAMPLER* sampler = g_sound->OpenNewSampler();
	if (sampler == NULL)
		return;

	if (instances < 0)
	{
//		sampler->stage = GSS_RELEASE;
		throw (wxString)_("regression - this should be impossible");
	}

	sampler->pipe = this;
	sampler->pipe_section = &m_attack;
	sampler->position = 0;

	GOrgueReleaseAlignTable::CopyTrackingInfo
		(sampler->release_tracker
		,m_attack.release_tracker_initial
		);

//	else
//	{
//		sampler->stage = GSS_ATTACK;
//		if (g_sound->HasRandomPipeSpeech() && !g_sound->windchests[m_WindchestGroup])
//			sampler->position = rand() & 0x78;
//	}

	sampler->fade = sampler->fademax = ra_amp;
	sampler->shift = ra_shift;
	sampler->time = organfile->GetElapsedTime();

	this->sampler = sampler;

	sampler->next = g_sound->windchests[m_WindchestGroup];
	g_sound->windchests[m_WindchestGroup] = sampler;

	if (instances == 0)
		instances++;

}

void GOrguePipe::SetOff()
{

	if (instances <= 0)
		return;

	instances--;

	if (m_loop.data == NULL)
		return;

	if (instances > 0)
		return;

	double vol = organfile->GetWindchest(m_WindchestGroup)->GetVolume();
	if (vol)
	{

		GO_SAMPLER* new_sampler = g_sound->OpenNewSampler();
		if (new_sampler != NULL)
		{

			new_sampler->pipe = this;
			new_sampler->pipe_section = &m_release;
			new_sampler->position = 0;
			new_sampler->shift = ra_shift;
			int time = organfile->GetElapsedTime() - sampler->time;
			new_sampler->time = organfile->GetElapsedTime();
			new_sampler->fademax = ra_amp;
			bool not_a_tremulant = (m_WindchestGroup >= organfile->GetTremulantCount());
			if (g_sound->HasScaledReleases() && not_a_tremulant)
			{
				if (time < 256)
					new_sampler->fademax = (ra_amp * (16384 + (time << 5))) >> 15;
				if (time < 1024)
					new_sampler->fadeout = 0x0001;
			}
			if (not_a_tremulant)
				new_sampler->fademax = lrint(vol * new_sampler->fademax);
			new_sampler->fadein = (new_sampler->fademax + 128) >> 8;
			if (new_sampler->fadein == 0)
				new_sampler->fadein--;
			new_sampler->faderemain = 512;	// 32768*65536 / 64*65536

			/* FIXME: this must be enabled again at some point soon */
			if (g_sound->HasReleaseAlignment() && (m_ra_table != NULL))
			{
				m_ra_table->SetupRelease(*new_sampler, *sampler);
			}
			else
			{
				new_sampler->position = 0; //m_release.offset;
				GOrgueReleaseAlignTable::CopyTrackingInfo
					(new_sampler->release_tracker
					,m_release.release_tracker_initial
					);
			}

			const int detached_windchest_index = organfile->GetTremulantCount();
			if (not_a_tremulant)
			{
				/* detached releases are enabled and the pipe was on a regular
				 * windchest. Play the release on the detached windchest */
				new_sampler->next = g_sound->windchests[detached_windchest_index];
				g_sound->windchests[detached_windchest_index] = new_sampler;
			}
			else
			{
				/* detached releases are disabled (or this isn't really a pipe)
				 * so put the release on the same windchest as the pipe (which
				 * means it will still be affected by tremulants - yuck). */
				new_sampler->next = g_sound->windchests[m_WindchestGroup];
				g_sound->windchests[m_WindchestGroup] = new_sampler;
			}

		}

	}

	/* The above code created a new sampler to playback the release, the
	 * following code takes the active sampler for this pipe (which will be
	 * in either the attack or loop section) and sets the fadeout property
	 * which will decay this portion of the pipe. The sampler will
	 * automatically be placed back in the pool when the fade restores to
	 * zero.
	 */
	assert(sampler);
	sampler->fadeout = (-ra_amp - 128) >> 8; /* recall that ra_amp is negative
	                                          * so this will actually be a
	                                          * positive number */
    if (!sampler->fadeout) /* ensure that the sample will fade out */
        sampler->fadeout++;
	this->sampler = 0;

}

void GOrguePipe::Set(bool on)
{
	if (m_ref)
	{
		m_ref->Set(on);
		return;
	}
	if (on)
		SetOn();
	else
		SetOff();
}

void GOrguePipe::LoadData()
{

#ifdef __VFD__
	int n=(((m_NumberOfLoadedPipes) << 15) / (int)(m_NumberOfPipes))/327;
	GOrgueLCD_WriteLineTwo(wxString::Format("Loading %d%%", n));
#endif

	if (m_filename.StartsWith(wxT("REF:")))
	{
		int manual, stop, pipe;
		sscanf(m_filename.mb_str() + 4, "%d:%d:%d", &manual, &stop, &pipe);
		if ((manual < organfile->GetFirstManualIndex()) || (manual > organfile->GetManualAndPedalCount()) ||
			(stop <= 0) || (stop > organfile->GetManual(manual)->GetStopCount()) ||
			(pipe <= 0) || (pipe > organfile->GetManual(manual)->GetStop(stop-1)->NumberOfLogicalPipes))
			throw (wxString)_("Invalid reference ") + m_filename;

		m_ref = organfile->GetManual(manual)->GetStop(stop-1)->GetPipe(pipe-1);

		return;
	}
	wxLogDebug(_("Loading file %s"), m_filename.c_str());

	GOrgueWave wave;
	wave.Open(m_filename);

	/* allocate data to work with */
	unsigned totalDataSize = wave.GetLength() * sizeof(wxInt16) * wave.GetChannels();
	wxInt16* data = (wxInt16*)malloc(totalDataSize);
	if (data == NULL)
		throw (wxString)_("< out of memory allocating wave");

	memset(&m_loop, 0, sizeof(m_loop));
	memset(&m_attack, 0, sizeof(m_attack));
	memset(&m_release, 0, sizeof(m_release));

	try
	{

		wave.ReadSamples(data, GOrgueWave::SF_SIGNEDSHORT, 44100);
		m_SampleRate = 44100;

		m_Channels = wave.GetChannels();
		if (m_Channels < 1 || m_Channels > 2)
			throw (wxString)_("< More than 2 channels in");

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

		if ((wave.GetNbLoops() > 0) && wave.HasReleaseMarker())
		{

			/* The wave has loops and a release marker so truncate the samples
			 * stored in the attack portion to the beginning of the loop.
			 */
			attackSamples = wave.GetLoopStartPosition();

			/* Get the loop parameters */
			unsigned loopStart = wave.GetLoopStartPosition();
			unsigned loopSamples = wave.GetLoopEndPosition() - loopStart + 1;
			unsigned loopSamplesInMem = loopSamples + SAMPLE_SLACK;
			assert(loopStart > 0);
			assert(wave.GetLoopEndPosition() > loopStart);

			/* Allocate memory for the loop, copy the loop into it and then
			 * copy some slack samples from the beginning of the loop onto
			 * the end to ensure correct operation of the sampler.
			 */
			m_loop.size = loopSamples * sizeof(wxInt16) * m_Channels;
			m_loop.data = (unsigned char*)malloc(loopSamplesInMem * sizeof(wxInt16) * m_Channels);
			if (m_loop.data == NULL)
				throw (wxString)_("< out of memory allocating loop");
			memcpy(m_loop.data,
				&data[loopStart * m_Channels],
				m_loop.size);
			memcpy(&m_loop.data[m_loop.size],
				&data[loopStart * m_Channels],
				loopSamplesInMem * sizeof(wxInt16) * m_Channels - m_loop.size);

			/* Get the release parameters from the wave file. */
			unsigned releaseOffset = wave.GetReleaseMarkerPosition();
			unsigned releaseSamples = wave.GetLength() - releaseOffset;
			unsigned releaseSamplesInMem = releaseSamples + SAMPLE_SLACK;

			/* Allocate memory for the release, copy the release into it and
			 * pad the slack samples with zeroes to ensure correct operation
			 * of the sampler.
			 */
			m_release.size = (int)releaseSamples * sizeof(wxInt16) * m_Channels;
			m_release.data = (unsigned char*)malloc(releaseSamplesInMem * sizeof(wxInt16) * m_Channels);
			if (m_release.data == NULL)
				throw (wxString)_("< out of memory allocating release");
			memcpy(m_release.data,
				&data[(wave.GetLength() - releaseSamples) * m_Channels],
				m_release.size);
			memset(&m_release.data[m_release.size],
				0,
				releaseSamplesInMem * sizeof(wxInt16) * m_Channels - m_release.size);

		}

		/* Allocate memory for the attack. */
		assert(attackSamples != 0);
		unsigned attackSamplesInMem = attackSamples + SAMPLE_SLACK;
		m_attack.size = (int)attackSamples * sizeof(wxInt16) * m_Channels;
		assert((unsigned)m_attack.size <= totalDataSize); /* can be equal for percussive samples */
		m_attack.data = (unsigned char*)malloc(attackSamplesInMem * sizeof(wxInt16) * m_Channels);
		if (m_attack.data == NULL)
			throw (wxString)_("< out of memory allocating attack");

		if (attackSamplesInMem <= wave.GetLength())
		{
			memcpy(m_attack.data,
				&data[0],
				attackSamplesInMem * sizeof(wxInt16) * m_Channels);
		}
		else
		{
			memset(
				m_attack.data,
				0,
				(attackSamplesInMem - wave.GetLength()) * sizeof(wxInt16) * m_Channels);
			memcpy(&m_attack.data[(attackSamplesInMem - wave.GetLength()) * sizeof(wxInt16) * m_Channels],
				&data[0],
				totalDataSize);
		}

		/* data is no longer needed */
		FREE_AND_NULL(data);

		/* Amplitude is the combination of global amplitude volume and the stop
		 * volume. 10000 would correspond to sample playback at normal volume.
		 */
		int amp = m_amplitude;
		ra_shift = 7;
		while (amp > 10000)
		{
			ra_shift--;
			amp >>= 1;
		}

		/* During normal playback, this value will be multipled with each
		 * sample (in the form of the "fade" parameter) fademax is also set
		 * to this value to specify a maximum multiplier that can be applied
		 * to keep the sample within the required headroom. */
		ra_amp = (amp << 15) / -10000;
		sampler = NULL;

		if (m_Channels == 1)
		{
			m_attack.type = AC_UNCOMPRESSED_MONO;
			m_loop.type = AC_UNCOMPRESSED_MONO;
			m_release.type = AC_UNCOMPRESSED_MONO;
		}
		else
		{
			m_attack.type = AC_UNCOMPRESSED_STEREO;
			m_loop.type = AC_UNCOMPRESSED_STEREO;
			m_release.type = AC_UNCOMPRESSED_STEREO;
		}

		m_attack.stage = GSS_ATTACK;
		m_loop.stage = GSS_LOOP;
		m_release.stage = GSS_RELEASE;

		if (wave.HasReleaseMarker())
			ComputeReleaseAlignmentInfo();

	}
	catch (wxString error)
	{
		wxLogError(_("caught exception: %s\n"), error.c_str());
		FREE_AND_NULL(data);
		FREE_AND_NULL(m_attack.data);
		FREE_AND_NULL(m_loop.data);
		FREE_AND_NULL(m_release.data);
		throw;
	}
	catch (...)
	{
		FREE_AND_NULL(data);
		FREE_AND_NULL(m_attack.data);
		FREE_AND_NULL(m_loop.data);
		FREE_AND_NULL(m_release.data);
		throw;
	}

}

short SynthTrem(double amp, double angle)
{
	return (short)(amp * sin(angle));
}

short SynthTrem(double amp, double angle, double fade)
{
	return (short)(fade * amp * sin(angle));
}

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

//FIXME: this function should not exist... it is here purely for legacy
//support in GOrgueSound::MIDIAllNotesOff()
void GOrguePipe::FastAbort()
{
	if (m_ref)
		m_ref->FastAbort();	
	if (instances > -1)
		instances = 0;
	sampler = 0;

}

wxString GOrguePipe::GetFilename()
{
	return m_filename;
}
