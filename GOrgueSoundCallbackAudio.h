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

#pragma once

#include "MMXSound.h"   // TODO: only if win32, etc.

double final_buff[1024 * 2];
float volume_buff[1024 * 2];
int g_buff[11][(1024 + 2) * 2];

int GOrgueSoundCallbackAudio(char* buffer, int bufferSize, void* WXUNUSED(userData))
{
	int i, j, k;

	if (organfile)
        organfile->m_elapsed = g_sound->sw.Time();

	std::vector<unsigned char> msg;
	for (j = 0; j < g_sound->n_midiDevices; j++)
	{
		if (!g_sound->b_midiDevices[j])
			continue;

		for(;;)
		{
			g_sound->midiDevices[j]->getMessage(&msg);
			if (msg.empty())
				break;
			GOrgueSoundCallbackMIDI(msg, j);
		}
	}

	if (!g_sound->b_active || !g_sound->samplers_count)
		GOrgueMemset(buffer, 0, bufferSize);
	if (!g_sound->b_active)
		return 0;

	if (g_sound->samplers_count > g_sound->meter_poly)
		g_sound->meter_poly = g_sound->samplers_count;

	bufferSize <<= 1;
	GOrgueMemset(final_buff, 0, bufferSize << 1);
	for (j = 0; j < organfile->NumberOfTremulants + 1 + organfile->NumberOfWindchestGroups; j++)
	{
		if (!g_sound->windchests[j])
			continue;

		GOrgueSampler* ptr = (GOrgueSampler*)(g_sound->windchests + j);

		int* this_buff;
		if (j < organfile->NumberOfTremulants)
			this_buff = g_buff[j + 1] + 2;
		else
		{
			this_buff = g_buff[0] + 2;
			register double d = 1.0, scale = 1.0/12700.0;
			for (i = 0; i < organfile->windchest[j].NumberOfEnclosures; i++)
			{
				register GOrgueEnclosure* enclosure = organfile->enclosure + (organfile->windchest[j].enclosure[i]);
				d *= (double)(enclosure->MIDIValue * (100 - enclosure->AmpMinimumLevel) + 127 * enclosure->AmpMinimumLevel) * scale;
			}
			organfile->windchest[j].m_Volume = d;
			d *= g_sound->volume;
			d *= 0.00000000059604644775390625;  // (2 ^ -24) / 100

			float f = d;
            GOrgueMemset(volume_buff, *(int*)&f, bufferSize);
		}

        GOrgueMemset(this_buff, j < organfile->NumberOfTremulants ? 0x800000 : 0, bufferSize);
		int *this_buff_end = this_buff + bufferSize;
		for (register GOrgueSampler* sampler = g_sound->windchests[j]; sampler; sampler = sampler->next)
		{
            if (g_sound->b_limit && g_sound->samplers_count >= g_sound->poly_soft && sampler->stage == 2 && organfile->m_elapsed - sampler->time > 250)
                sampler->fadeout = 4;

            asm("\
                movd %19,%%mm7         \n\
                movd %2,%%mm4          \n\
                movq %7,%%mm0          \n\
                paddd -8(%12,%11),%%mm0\n\
                movl $0,%3             \n\
                movq %%mm0,-8(%12,%11) \n\
                movq %8,%%mm2          \n\
                movq %9,%%mm3          \n\
                movd %10,%%mm5         \n\
                punpcklwd %%mm4,%%mm4  \n\
                psllq $16,%%mm5        \n\
                punpckldq %%mm4,%%mm4  \n\
                punpckldq %%mm5,%%mm5  \n\
                orl %10,%10            \n\
                jnz NewSampleTypeVYFY  \n\
                cmpl $-32768,%2        \n\
                jnz NewSampleTypeVYFN  \n\
                " ASM_CORE(VNFN) "     \n\
                " ASM_CORE(VYFY) "     \n\
                " ASM_CORE(VYFN) "     \n\
            GOrgueStage2:                  \n\
                movl $0,%13            \n\
            GOrgueDone:                    \n\
                movd %%mm4,%2          \n\
                movq %%mm6,%7          \n\
                movq %%mm2,%8          \n\
                movq %%mm3,%9          \n\
            "

            : /* ESI % 0 */ "=S" (sampler->ptr),
              /* ECX % 1 */ "=c" (sampler->current),
              /*     % 2 */ "=m" (sampler->fade),
              /*     % 3 */ "=m" (sampler->offset),
              /*     % 4 */ "=m" (sampler->type),
              /*     % 5 */ "=m" (sampler->stage),
              /*     % 6 */ "=m" (sampler->overflowing),
              /*     % 7 */ "=m" (sampler->overflow),
              /*     % 8 */ "=m" (sampler->f),
              /*     % 9 */ "=m" (sampler->v)
            : /*     %10 */  "a" (sampler->fadein + sampler->fadeout),
              /* EDX %11 */  "d" (sampler->offset + sampler->overflowing - (bufferSize << 2)),
              /* EDI %12 */  "D" (this_buff_end),
              /*     %13 */  "m" (sampler->pipe),
              /*     %14 */  "m" (sampler->pipe->ptr[1]),
              /*     %15 */  "m" (sampler->pipe->offset[1]),
              /*     %16 */  "m" (sampler->pipe->types[1]),
              /*     %17 */  "m" (sampler->pipe->f[1][0]),
              /*     %18 */  "m" (sampler->pipe->v[1][0]),
              /*     %19 */  "m" (sampler->shift),
                             "S" (sampler->ptr),
                             "c" (sampler->current)
            );

            sampler->fade >>= 16;
			if (!sampler->pipe && (!sampler->overflowing || !sampler->overflow))
				sampler->fade = 0;
			else if (sampler->fade < sampler->fademax)
				sampler->fadein = 0;
			else if (sampler->fadein)
			{
				sampler->faderemain -= bufferSize >> 2;
				if (sampler->faderemain <= 0)
					sampler->fadein = 0;
			}

			if (!sampler->pipe || !sampler->fade)
			{
				ptr->next = sampler->next;
				g_sound->samplers_open[--(g_sound->samplers_count)] = sampler;
			}
			else
				ptr = sampler;
		}

		asm("emms");

		if (j >= organfile->NumberOfTremulants)
		{
			register int *ptr;
			for (i = 0; i < organfile->windchest[j].NumberOfTremulants; i++)
			{
				if (!g_sound->windchests[organfile->windchest[j].tremulant[i]])
					continue;
                ptr = g_buff[organfile->windchest[j].tremulant[i] + 1] + 2;
                for (k = 0; k < bufferSize; k++)
                #ifdef linux
                    volume_buff[k] *= scalb(ptr[k], -23);
                #endif
                #ifdef _WIN32
                    volume_buff[k] *= _scalb(ptr[k], -23);
                #endif
			}

			ptr = this_buff;
			for (k = 0; k < bufferSize; k++)
			{
			    register double d = this_buff[k];
			    d *= volume_buff[k];
                final_buff[k] += d;
			}
		}
	}

    register double clamp_min = -1.0, clamp_max = 1.0;
    for (k = 0; k < bufferSize; k += 2)
    {
        register double d;
        d = final_buff[k + 0];
        if (d < clamp_min)
        	d = clamp_min;
        else if (d > clamp_max)
        	d = clamp_max;
        d = fabs(d);
        if (d > g_sound->meter_left)
        	g_sound->meter_left = d;

        d = final_buff[k + 1];
        if (d < clamp_min)
        	d = clamp_min;
        else if (d > clamp_max)
        	d = clamp_max;
        d = fabs(d);
        if (d > g_sound->meter_right)
        	g_sound->meter_right = d;

    }

	if (g_sound->f_output)
	{
		fwrite(buffer, sizeof(float), bufferSize, g_sound->f_output);
		if (g_sound->b_stoprecording)
			g_sound->CloseWAV();
	}

	g_sound->meter_counter += bufferSize;
	if (g_sound->meter_counter >= 6144)	// update 44100 / (N / 2) = ~14 times per second
	{
        int n = 0;
        // polyphony
        n |= ( 33 * g_sound->meter_poly ) / g_sound->polyphony;
        n <<= 8;
        // right channel
        n |= lrint(32.50000000000001 * g_sound->meter_right);
        n <<= 8;
        // left  channel
        n |= lrint(32.50000000000001 * g_sound->meter_left );

        wxCommandEvent event(wxEVT_METERS, 0);
        event.SetInt(n);
        ::wxGetApp().frame->AddPendingEvent(event);

        g_sound->meter_counter = g_sound->meter_poly = 0;
        g_sound->meter_left = g_sound->meter_right = 0.0;
	}

	return 0;
}
