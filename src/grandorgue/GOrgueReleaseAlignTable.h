/*
 * GOrgueReleaseAlignTable.h
 *
 *  Created on: 26/04/2011
 *      Author: nappleton
 */

#ifndef GORGUERELEASEALIGNTABLE_H_
#define GORGUERELEASEALIGNTABLE_H_

#define PHASE_ALIGN_DERIVATIVES    2
#define PHASE_ALIGN_AMPLITUDES     32
#define PHASE_ALIGN_MIN_FREQUENCY  20 /* Hertz */

#include "GOrgueSoundTypes.h"

class wxInputStream;
class wxOutputStream;

class GOrgueReleaseAlignTable
{

private:

	unsigned int m_Channels;
	int m_PhaseAlignMaxAmplitude;
	int m_PhaseAlignMaxDerivative;
	int m_PositionEntries[PHASE_ALIGN_DERIVATIVES][PHASE_ALIGN_AMPLITUDES];
	int m_HistoryEntries[PHASE_ALIGN_DERIVATIVES][PHASE_ALIGN_AMPLITUDES][BLOCK_HISTORY * MAX_OUTPUT_CHANNELS];

public:

	GOrgueReleaseAlignTable();
	~GOrgueReleaseAlignTable();

	bool Load(wxInputStream* cache);
	bool Save(wxOutputStream* cache);

	void ComputeTable
		(const AUDIO_SECTION_T& m_release
		,const int phase_align_max_amplitude
		,const int phase_align_max_derivative
		,const unsigned int sample_rate
		,const unsigned int channels
		);

	void SetupRelease
		(GO_SAMPLER_T& release_sampler
		,const GO_SAMPLER_T& old_sampler
		);

};

#endif /* GORGUERELEASEALIGNTABLE_H_ */
