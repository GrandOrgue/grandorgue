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

#ifndef GORGUEPIPE_H
#define GORGUEPIPE_H

#include <wx/wx.h>
#include "GOrgueSound.h"

class GOrgueTremulant;

#define PHASE_ALIGN_DERIVATIVES    2
#define PHASE_ALIGN_AMPLITUDES     32
#define PHASE_ALIGN_MIN_FREQUENCY  20 /* Hertz */

typedef enum
{
	AC_COMPRESSED_MONO = 0,
	AC_UNCOMPRESSED_MONO = 1,
	AC_COMPRESSED_STEREO = 2,
	AC_UNCOMPRESSED_STEREO = 3
} AUDIO_SECTION_TYPE;

typedef struct
{

	/* Size of the section in BYTES */
	int size;

	/* Type of the data which is stored in the data pointer */
	AUDIO_SECTION_TYPE type;

	/* The starting sample and derivatives for each channel (used in the
	 * compression and release-alignment schemes */
	int start_f[MAX_OUTPUT_CHANNELS];
	int start_v[MAX_OUTPUT_CHANNELS];

	/* Pointer to (size) bytes of data encoded in the format (type) */
	unsigned char* data;

} AUDIO_SECTION;

class GOrguePipe
{

private:

	void SetOn();
	void SetOff();

	int m_Channels;
	int m_SampleRate;
	int m_PhaseAlignMaxAmplitude;
	int m_PhaseAlignMaxDerivative;

	int m_PhaseAlignmentTable[PHASE_ALIGN_DERIVATIVES][PHASE_ALIGN_AMPLITUDES];
	int m_PhaseAlignmentTable_f[PHASE_ALIGN_DERIVATIVES][PHASE_ALIGN_AMPLITUDES][MAX_OUTPUT_CHANNELS];
	int m_PhaseAlignmentTable_v[PHASE_ALIGN_DERIVATIVES][PHASE_ALIGN_AMPLITUDES][MAX_OUTPUT_CHANNELS];

	void GetMaxAmplitudeAndDerivative(AUDIO_SECTION& section, int& runningMaxAmplitude, int& runningMaxDerivative);
	void ComputeReleaseAlignmentInfo();
	void SetupReleaseSamplerPosition(GO_SAMPLER& newReleaseSampler);

public:

	~GOrguePipe();
	GOrguePipe();

	void Set(bool on);
	void LoadFromFile(const wxString& filename, int amp);
	void CreateFromTremulant(GOrgueTremulant* tremulant);

	float pitch;
	GO_SAMPLER* sampler;
	int instances;

	/* states which windchest this pipe belongs to... groups from
	 * 0 to GetTremulantCount-1 are purely there for tremulants. */
	int WindchestGroup;
	int ra_amp;
	int ra_shift;

	AUDIO_SECTION m_attack;
	AUDIO_SECTION m_loop;
	AUDIO_SECTION m_release;

};

#endif
