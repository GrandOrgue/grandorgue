/*
 * GOrgueRtHelpers.h
 *
 *  Created on: Jun 19, 2011
 *      Author: nick
 */

#ifndef GORGUERTHELPERS_H_
#define GORGUERTHELPERS_H_

#include <wx/wx.h>
#include "RtAudio.h"

class GOrgueRtHelpers
{

private:

	static void GetDirectSoundConfig(const int latency, unsigned *nb_buffers, unsigned *buffer_size);
	static void GetAsioSoundConfig(const int latency, unsigned *nb_buffers, unsigned *buffer_size);
	static void GetJackSoundConfig(const int latency, unsigned *nb_buffers, unsigned *buffer_size);
	static void GetUnknownSoundConfig(const int latency, unsigned *nb_buffers, unsigned *buffer_size);

public:

	static void GetBufferConfig
		(const RtAudio::Api   rt_api
		,const unsigned       latency_ms
		,unsigned            *nb_buffers
		,unsigned            *buffer_size_samples
		);

	static const wxChar* GetApiName(const RtAudio::Api api);
	static const wxChar* GetAudioFormatName(const int rt_audio_format);

};

#endif /* GORGUERTHELPERS_H_ */
