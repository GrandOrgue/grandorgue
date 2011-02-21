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
#include "GOrgueWindchest.h"
#include "GrandOrgueFile.h"

extern GOrgueSound* g_sound;
extern GrandOrgueFile* organfile;

void GOrguePipe::Set(bool on)
{
	if (on)
	{
		if (instances < 1)
		{
			GOrgueSampler* sampler = g_sound->OpenNewSampler();
			if (sampler == NULL)
				return;

			if (instances < 0)
				sampler->stage = 2;
			else if (g_sound->HasRandomPipeSpeech() && !g_sound->windchests[WindchestGroup])
				sampler->offset = rand() & 0x78;
			sampler->type = types[0];
			sampler->fade = sampler->fademax = ra_amp;
			sampler->current = offset[0];
			sampler->ptr = ptr[0];
			sampler->pipe = this;
			sampler->shift = ra_shift;
			sampler->f = *(wxInt64*)(f[0]);
			sampler->v = *(wxInt64*)(v[0]);
			sampler->time = organfile->GetElapsedTime();
			this->sampler = sampler;

			sampler->next = g_sound->windchests[WindchestGroup];
			g_sound->windchests[WindchestGroup] = sampler;
		}
		if (instances > -1)
			instances++;
	}
	else
	{
		if (instances > 0)
		{
			if (instances == 1)
			{
				if (organfile->GetWindchest(WindchestGroup)->m_Volume)
				{
					GOrgueSampler* sampler = g_sound->OpenNewSampler();
					if (sampler != NULL)
					{
						sampler->current = offset[2];
						sampler->ptr = ptr[2];
						sampler->pipe = this;
						sampler->shift = ra_shift;
						sampler->stage = 2;
						sampler->type = types[2];
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
							sampler->fademax = lrint(((double)sampler->fademax) * organfile->GetWindchest(WindchestGroup)->m_Volume);
						sampler->fadein = (sampler->fademax + 128) >> 8;
						if (!sampler->fadein)
							sampler->fadein--;
						sampler->faderemain = 512;	// 32768*65536 / 64*65536

						if (g_sound->HasReleaseAlignment())
						{
							int index = ra_getindex((short*)&this->sampler->f, (short*)&this->sampler->v);
							sampler->current = ra_offset[index];
							sampler->f = *(wxInt64*)(ra_f[index]);
							sampler->v = *(wxInt64*)(ra_v[index]);
						}
						else
						{
							sampler->current = offset[2];
							sampler->f = *(wxInt64*)(f[2]);
							sampler->v = *(wxInt64*)(v[2]);
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
			instances--;
		}
	}
}


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
