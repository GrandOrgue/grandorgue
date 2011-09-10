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
	int m_PhaseAlignmentTable[PHASE_ALIGN_DERIVATIVES][PHASE_ALIGN_AMPLITUDES];
	int m_PhaseAlignmentTable_f[PHASE_ALIGN_DERIVATIVES][PHASE_ALIGN_AMPLITUDES];
	int m_PhaseAlignmentTable_v[PHASE_ALIGN_DERIVATIVES][PHASE_ALIGN_AMPLITUDES];

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



	static void UpdateTrackingInfo
		(GO_RELEASE_TRACKING_INFO& track_info
		,const unsigned nb_samples
		,const int* samples
		);

	static void CopyTrackingInfo
		(GO_RELEASE_TRACKING_INFO& dest
		,const GO_RELEASE_TRACKING_INFO& source
		);


};

inline
void GOrgueReleaseAlignTable::UpdateTrackingInfo
	(GO_RELEASE_TRACKING_INFO& track_info
	,const unsigned nb_samples
	,const int* samples
	)
{

	if (nb_samples < 1)
		return;

	if (nb_samples < 2)
	{
		track_info.f[1] = track_info.f[0];
		track_info.f[0] = samples[0];
		return;
	}

	track_info.f[1] = samples[nb_samples-2];
	track_info.f[0] = samples[nb_samples-1];

}

inline
void GOrgueReleaseAlignTable::CopyTrackingInfo
	(GO_RELEASE_TRACKING_INFO& dest
	,const GO_RELEASE_TRACKING_INFO& source
	)
{

	memcpy(&dest, &source, sizeof(source));

}

#endif /* GORGUERELEASEALIGNTABLE_H_ */
