/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2011 GrandOrgue contributors (see AUTHORS)
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 * MA 02111-1307, USA.
 */

#ifndef GORGUESTOP_H
#define GORGUESTOP_H

#include <vector>
#include "ptrvector.h"
#include <wx/wx.h>
#include "IniFileConfig.h"
#include "GOrgueDrawStop.h"

class GOrguePipe;

class GOrgueStop : public GOrgueDrawstop
{
private:
	ptr_vector<GOrguePipe> m_Pipes;
	std::vector<unsigned> m_KeyState;
	unsigned m_ManualNumber;
	bool m_Percussive;
	float m_AmplitudeLevel;
	float m_DefaultAmplitude;
	unsigned m_FirstAccessiblePipeLogicalPipeNumber;
	unsigned m_FirstAccessiblePipeLogicalKeyNumber;
	unsigned m_NumberOfAccessiblePipes;
	unsigned m_WindchestGroup;

public:
	GOrgueStop(GrandOrgueFile* organfile, unsigned manual_number);
	GOrguePipe* GetPipe(unsigned index);
	unsigned GetPipeCount();
	void Load(IniFileConfig& cfg, wxString group);
	void Save(IniFileConfig& cfg, bool prefix);
	void Set(bool on);
	void SetKey(unsigned note, int on);
	void Abort();
	void PreparePlayback();
	~GOrgueStop(void);

	unsigned GetFirstAccessiblePipeLogicalPipeNumber() const;
	unsigned GetFirstAccessiblePipeLogicalKeyNumber() const;
	unsigned GetNbAccessiblePipes() const;
	unsigned IsAuto() const;
	float GetAmplitude() const;
	float GetDefaultAmplitude() const;
	void SetAmplitude(float amp);

};

#endif /* GORGUESTOP_H_ */
