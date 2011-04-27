/*
 * GOrgueReleaseAlignTable.h
 *
 *  Created on: 26/04/2011
 *      Author: nappleton
 */

#ifndef GORGUERELEASEALIGNTABLE_H_
#define GORGUERELEASEALIGNTABLE_H_

#include "GOrgueSound.h"

#define PHASE_ALIGN_DERIVATIVES    2
#define PHASE_ALIGN_AMPLITUDES     32
#define PHASE_ALIGN_MIN_FREQUENCY  20 /* Hertz */

/* data structure required to support release alignment tracking. */
typedef struct
{
	int f[4];
} GO_RELEASE_TRACKER;

class GOrgueReleaseAlignTable
{

private:

	unsigned int m_Channels;
	int m_PhaseAlignMaxAmplitude;
	int m_PhaseAlignMaxDerivative;
	int m_PhaseAlignmentTable[PHASE_ALIGN_DERIVATIVES][PHASE_ALIGN_AMPLITUDES];
	int m_PhaseAlignmentTable_f[PHASE_ALIGN_DERIVATIVES][PHASE_ALIGN_AMPLITUDES][MAX_OUTPUT_CHANNELS];
	int m_PhaseAlignmentTable_v[PHASE_ALIGN_DERIVATIVES][PHASE_ALIGN_AMPLITUDES][MAX_OUTPUT_CHANNELS];

public:

	GOrgueReleaseAlignTable();
	~GOrgueReleaseAlignTable();

	void ComputeTable
		(const AUDIO_SECTION& m_release
		,const int phase_align_max_amplitude
		,const int phase_align_max_derivative
		,const unsigned int sample_rate
		,const unsigned int channels
		);

	void SetupRelease
		(GO_SAMPLER& release_sampler
		,const GO_SAMPLER& old_sampler
		);



};


#endif /* GORGUERELEASEALIGNTABLE_H_ */
