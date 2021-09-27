/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUERELEASEALIGNTABLE_H_
#define GORGUERELEASEALIGNTABLE_H_

class GOAudioSection;
class GOrgueCache;
class GOrgueCacheWriter;
typedef struct audio_section_stream_s audio_section_stream;

#define PHASE_ALIGN_DERIVATIVES    2
#define PHASE_ALIGN_AMPLITUDES     32
#define PHASE_ALIGN_MIN_FREQUENCY  20 /* Hertz */

class GOrgueReleaseAlignTable
{

private:

	int m_PhaseAlignMaxAmplitude;
	int m_PhaseAlignMaxDerivative;
	int m_PositionEntries[PHASE_ALIGN_DERIVATIVES][PHASE_ALIGN_AMPLITUDES];

public:

	GOrgueReleaseAlignTable();
	~GOrgueReleaseAlignTable();

	bool Load(GOrgueCache& cache);
	bool Save(GOrgueCacheWriter& cache);

	void ComputeTable
		(const GOAudioSection &m_release
		,int                   phase_align_max_amplitude
		,int                   phase_align_max_derivative
		,unsigned int          sample_rate
		,unsigned              start_position
		);

	void SetupRelease
		(audio_section_stream       &release_sampler
		,const audio_section_stream &old_sampler
		) const;

};

#endif /* GORGUERELEASEALIGNTABLE_H_ */
