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

	int f;
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

	memset(m_PhaseAlignmentTable, 0, sizeof(m_PhaseAlignmentTable));
	memset(m_PhaseAlignmentTable_f, 0, sizeof(m_PhaseAlignmentTable_f));
	memset(m_PhaseAlignmentTable_v, 0, sizeof(m_PhaseAlignmentTable_v));

	/* We will use a short portion of the release to analyse to get the
	 * release offset table. This length is defined by the
	 * PHASE_ALIGN_MIN_FREQUENCY macro and should be set to the lowest
	 * frequency pipe you would ever expect... if this length is greater
	 * than the length of the release, truncate it */
	unsigned searchLen = m_SampleRate / PHASE_ALIGN_MIN_FREQUENCY;
	unsigned releaseLen = m_release.size / (m_Channels * sizeof(wxInt16));
	if (searchLen > releaseLen)
		searchLen = releaseLen;

	/* If number of samples in the release is not enough to fill the release
	 * table, abort - release alignment probably wont help. */
	if (searchLen < PHASE_ALIGN_AMPLITUDES * PHASE_ALIGN_DERIVATIVES * 2)
		return;

	/* Find the maximum amplitude and derivative of the waveform */
	m_PhaseAlignMaxAmplitude = 0;
	m_PhaseAlignMaxDerivative = 0;
	GetMaxAmplitudeAndDerivative(m_attack,  m_PhaseAlignMaxAmplitude, m_PhaseAlignMaxDerivative);
	GetMaxAmplitudeAndDerivative(m_loop,    m_PhaseAlignMaxAmplitude, m_PhaseAlignMaxDerivative);
	GetMaxAmplitudeAndDerivative(m_release, m_PhaseAlignMaxAmplitude, m_PhaseAlignMaxDerivative);

	assert(m_PhaseAlignMaxDerivative != 0);
	assert(m_PhaseAlignMaxAmplitude != 0);

	/* Generate the release table using the small portion of the release... */
	bool found[PHASE_ALIGN_DERIVATIVES][PHASE_ALIGN_AMPLITUDES];
	memset(found, 0, sizeof(found));
	int f[MAX_OUTPUT_CHANNELS];
	int v[MAX_OUTPUT_CHANNELS];
	for (unsigned i = 0; i < searchLen; i++)
	{

		/* Store previous values */
		int f_p[MAX_OUTPUT_CHANNELS];
		memcpy(f_p, f, sizeof(int) * MAX_OUTPUT_CHANNELS);

		int fsum = 0;
		for (unsigned int j = 0; j < m_Channels; j++)
		{
			f[j] = *((wxInt16*)&m_release.data[(i * m_Channels + j) * sizeof(wxInt16)]);
			fsum += f[j];
		}

		if (i == 0)
			continue;

		int vsum = 0;
		for (unsigned int j = 0; j < m_Channels; j++)
		{
			v[j] = f[j] - f_p[j];
			vsum += v[j];
		}

		/* Bring v into the range -1..2*m_PhaseAlignMaxDerivative-1 */
		int v_mod = vsum + m_PhaseAlignMaxDerivative - 1;
		int derivIndex = (PHASE_ALIGN_DERIVATIVES * v_mod) / (2 * m_PhaseAlignMaxDerivative);

		/* Bring f into the range -1..2*m_PhaseAlignMaxAmplitude-1 */
		int f_mod = fsum + m_PhaseAlignMaxAmplitude - 1;
		int ampIndex = (PHASE_ALIGN_AMPLITUDES * f_mod) / (2 * m_PhaseAlignMaxAmplitude);

		/* Store this release point if it was not already found */
		assert((derivIndex >= 0) && (derivIndex < PHASE_ALIGN_DERIVATIVES));
		assert((ampIndex >= 0)   && (ampIndex < PHASE_ALIGN_AMPLITUDES));
		if (!found[derivIndex][ampIndex])
		{
			m_PhaseAlignmentTable[derivIndex][ampIndex] = i * m_Channels * sizeof(wxInt16);
			for (unsigned int k = 0; k < m_Channels; k++)
			{
				m_PhaseAlignmentTable_f[derivIndex][ampIndex][k] = f[k];
				m_PhaseAlignmentTable_v[derivIndex][ampIndex][k] = v[k];
			}
			found[derivIndex][ampIndex] = true;
		}

	}

#ifndef NDEBUG
#ifdef PALIGN_DEBUG
	/* print some phase debugging information */
	for (unsigned int i = 0; i < PHASE_ALIGN_DERIVATIVES; i++)
	{
		printf("deridx: %d\n", i);
		for (unsigned int j = 0; j < PHASE_ALIGN_AMPLITUDES; j++)
			if (found[i][j])
				printf("  idx %d: found\n", j);
			else
				printf("  idx %d: not found\n", j);

	}
#endif
#endif

	/* Phase 2, if there are any entries in the table which were not found,
	 * attempt to fill them with the nearest available value. */
	for (int i = 0; i < PHASE_ALIGN_DERIVATIVES; i++)
		for (int j = 0; j < PHASE_ALIGN_AMPLITUDES; j++)
			if (!found[i][j])
			{
				bool foundsecond = false;
				for (int l = 0; (l < PHASE_ALIGN_DERIVATIVES) && (!foundsecond); l++)
					for (int k = 0; (k < PHASE_ALIGN_AMPLITUDES) && (!foundsecond); k++)
					{
						foundsecond = true;
						int sl = (l + 1) / 2;
						if ((sl & 1) == 0)
							sl = -sl;
						if ((i + sl < PHASE_ALIGN_DERIVATIVES) && (i + sl >= 0))
						{
							if ((j + k < PHASE_ALIGN_AMPLITUDES) && (found[i + sl][j + k]))
							{
								m_PhaseAlignmentTable[i][j] = m_PhaseAlignmentTable[i + sl][j + k];
								for (unsigned int z = 0; z < m_Channels; z++)
								{
									m_PhaseAlignmentTable_f[i][j][z] = m_PhaseAlignmentTable_f[i + sl][j + k][z];
									m_PhaseAlignmentTable_v[i][j][z] = m_PhaseAlignmentTable_v[i + sl][j + k][z];
								}
							}
							else if ((j - k >= 0) && (found[i + sl][j - k]))
							{
								m_PhaseAlignmentTable[i][j] = m_PhaseAlignmentTable[i + sl][j - k];
								for (unsigned int z = 0; z < m_Channels; z++)
								{
									m_PhaseAlignmentTable_f[i][j][z] = m_PhaseAlignmentTable_f[i + sl][j - k][z];
									m_PhaseAlignmentTable_v[i][j][z] = m_PhaseAlignmentTable_v[i + sl][j - k][z];
								}
							}
							else
							{
								foundsecond = false;
							}
						}
						else
						{
							foundsecond = false;
						}
					}

				assert(foundsecond);
				foundsecond = false;

			}

}

void GOrguePipe::SetupReleaseSamplerPosition(GO_SAMPLER& newReleaseSampler)
{

	assert(sampler != NULL);

	/* Get combined release f's and v's
	 * TODO: it might be good to check that the compiler
	 * un-rolls this loop */
	int v_mod = 0;
	int f_mod = 0;
	for (unsigned i = 0; i < MAX_OUTPUT_CHANNELS; i++)
	{
		v_mod += sampler->v[i];
		f_mod += sampler->f[i];
	}

	/* Bring f and v into the range -1..2*m_PhaseAlignMaxDerivative-1 */
	v_mod += (m_PhaseAlignMaxDerivative - 1);
	f_mod += (m_PhaseAlignMaxAmplitude - 1);

	int derivIndex = m_PhaseAlignMaxDerivative ?
			(PHASE_ALIGN_DERIVATIVES * v_mod) / (2 * m_PhaseAlignMaxDerivative) :
			PHASE_ALIGN_DERIVATIVES / 2;

	assert((derivIndex >= 0) && (derivIndex < PHASE_ALIGN_DERIVATIVES));

	/* Bring f into the range -1..2*m_PhaseAlignMaxAmplitude-1 */
	int ampIndex = m_PhaseAlignMaxAmplitude ?
			(PHASE_ALIGN_AMPLITUDES * f_mod) / (2 * m_PhaseAlignMaxAmplitude) :
			PHASE_ALIGN_AMPLITUDES / 2;

	assert((ampIndex >= 0) && (ampIndex < PHASE_ALIGN_AMPLITUDES));

	/* Store this release point if it was not already found */
	newReleaseSampler.position  = m_PhaseAlignmentTable[derivIndex][ampIndex];
	for (unsigned int i = 0; i < MAX_OUTPUT_CHANNELS; i++)
	{
		newReleaseSampler.f[i] = m_PhaseAlignmentTable_f[derivIndex][ampIndex][i];
		newReleaseSampler.v[i] = m_PhaseAlignmentTable_v[derivIndex][ampIndex][i];
	}

#ifndef NDEBUG
#ifdef PALIGN_DEBUG
	printf("setup release using alignment:\n");
	printf("  pos:    %d\n", newReleaseSampler.position);
	printf("  derIdx: %d\n", derivIndex);
	printf("  ampIdx: %d\n", ampIndex);
#endif
#endif

}


GOrguePipe::~GOrguePipe()
{
	FREE_AND_NULL(m_attack.data);
	FREE_AND_NULL(m_loop.data);
	FREE_AND_NULL(m_release.data);
}

GOrguePipe::GOrguePipe()
{

	memset(m_PhaseAlignmentTable, 0, sizeof(m_PhaseAlignmentTable));
	memset(m_PhaseAlignmentTable_f, 0, sizeof(m_PhaseAlignmentTable_f));
	memset(m_PhaseAlignmentTable_v, 0, sizeof(m_PhaseAlignmentTable_v));
	m_PhaseAlignMaxAmplitude = 0;
	m_PhaseAlignMaxDerivative = 0;

	memset(&m_attack, 0, sizeof(m_attack));
	memset(&m_loop, 0, sizeof(m_loop));
	memset(&m_release, 0, sizeof(m_release));

	pitch = 0;
	sampler = NULL;
	instances = 0;
	WindchestGroup = 0;
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
		sampler->stage = GSS_RELEASE;
	}
	else
	{
		sampler->stage = GSS_ATTACK;
//		if (g_sound->HasRandomPipeSpeech() && !g_sound->windchests[WindchestGroup])
//			sampler->position = rand() & 0x78;
	}

	sampler->type = m_attack.type;
	sampler->fade = sampler->fademax = ra_amp;

	sampler->position = 0;// m_attack.offset;
	sampler->ptr = m_attack.data;
	sampler->pipe = this;
	sampler->shift = ra_shift;
//	memcpy(sampler->f, m_attack.start_f, MAX_OUTPUT_CHANNELS * sizeof(sampler->f[0]));
//	memcpy(sampler->v, m_attack.start_v, MAX_OUTPUT_CHANNELS * sizeof(sampler->v[0]));
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

		GO_SAMPLER* new_sampler = g_sound->OpenNewSampler();
		if (new_sampler != NULL)
		{

			new_sampler->position = 0;
			new_sampler->ptr = m_release.data;
			new_sampler->pipe = this;
			new_sampler->shift = ra_shift;
			new_sampler->stage = GSS_RELEASE;
			new_sampler->type = m_release.type;
			int time = organfile->GetElapsedTime() - sampler->time;
			new_sampler->time = organfile->GetElapsedTime();
			new_sampler->fademax = ra_amp;
			bool not_a_tremulant = (WindchestGroup >= organfile->GetTremulantCount());
			if (g_sound->HasScaledReleases() && not_a_tremulant)
			{
				if (time < 256)
					new_sampler->fademax = (ra_amp * (16384 + (time << 5))) >> 15;
				if (time < 1024)
					new_sampler->fadeout = 0x0001;
			}
			if (g_sound->b_detach && not_a_tremulant)
				new_sampler->fademax = lrint(vol * new_sampler->fademax);
			new_sampler->fadein = (new_sampler->fademax + 128) >> 8;
			if (new_sampler->fadein == 0)
				new_sampler->fadein--;
			new_sampler->faderemain = 512;	// 32768*65536 / 64*65536

			/* FIXME: this must be enabled again at some point soon */
			if (g_sound->HasReleaseAlignment())
			{
				SetupReleaseSamplerPosition(*new_sampler);
			}
			else
			{
				new_sampler->position = 0; //m_release.offset;
				memcpy(new_sampler->f, m_release.start_f, MAX_OUTPUT_CHANNELS * sizeof(new_sampler->f[0]));
				memcpy(new_sampler->v, m_release.start_v, MAX_OUTPUT_CHANNELS * sizeof(new_sampler->v[0]));
			}

			const int detached_windchest_index = organfile->GetTremulantCount();
			if (g_sound->b_detach && not_a_tremulant)
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
				new_sampler->next = g_sound->windchests[WindchestGroup];
				g_sound->windchests[WindchestGroup] = new_sampler;
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
	unsigned totalDataSize = wave.GetLength() * sizeof(wxInt16) * wave.GetChannels();
	wxInt16* data = (wxInt16*)malloc(totalDataSize);
	if (data == NULL)
		throw (char*)"< out of memory allocating wave";

	try
	{

		wave.ReadSamples(data, GOrgueWave::SF_SIGNEDSHORT, 44100);
		m_SampleRate = 44100;

		m_Channels = wave.GetChannels();
		if (m_Channels < 1 || m_Channels > 2)
			throw (char*)"< More than 2 channels in";

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
				throw (char*)"< out of memory allocating loop";
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
				throw (char*)"< out of memory allocating release";
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
		assert(m_attack.size < totalDataSize);
		m_attack.data = (unsigned char*)malloc(attackSamplesInMem * sizeof(wxInt16) * m_Channels);
		if (m_attack.data == NULL)
			throw (char*)"< out of memory allocating attack";

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

		ComputeReleaseAlignmentInfo();

	}
	catch (char* error)
	{
		fprintf(stderr, "caught exception: %s\n", error);
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

void GOrguePipe::CreateFromTremulant(GOrgueTremulant* tremulant)
{

	static const int amp = 10000;
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

		m_Channels = 1;
		m_SampleRate = 44100;

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
		ra_amp = (amp << 15) / -10000;
		sampler = NULL;
		m_attack.type = AC_UNCOMPRESSED_MONO;
		m_loop.type = AC_UNCOMPRESSED_MONO;
		m_release.type = AC_UNCOMPRESSED_MONO;

		ComputeReleaseAlignmentInfo();

	}
	catch (...)
	{
		FREE_AND_NULL(m_attack.data);
		FREE_AND_NULL(m_loop.data);
		FREE_AND_NULL(m_release.data);
		throw;
	}

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

