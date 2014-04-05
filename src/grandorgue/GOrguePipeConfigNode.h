/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2014 GrandOrgue contributors (see AUTHORS)
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
 */

#ifndef GORGUEPIPECONFIGNODE_H
#define GORGUEPIPECONFIGNODE_H

#include "GOrguePipeConfig.h"
#include "GOrgueSaveableObject.h"

class GOrguePipeConfigNode : private GOrgueSaveableObject
{
private:
	GrandOrgueFile* m_organfile;
	GOrguePipeConfigNode* m_parent;
	GOrguePipeConfig m_PipeConfig;
	wxString m_Name;

	void Save(GOrgueConfigWriter& cfg);

public:
	GOrguePipeConfigNode(GOrguePipeConfigNode* parent, GrandOrgueFile* organfile, GOrguePipeUpdateCallback* callback);
	virtual ~GOrguePipeConfigNode();

	void Load(GOrgueConfigReader& cfg, wxString group, wxString prefix);

	const wxString& GetName();
	void SetName(wxString name);

	GOrguePipeConfig& GetPipeConfig();

	void ModifyTuning(float diff);

	float GetEffectiveAmplitude();
	float GetEffectiveGain();
	float GetEffectiveTuning();
	float GetDefaultTuning();

	unsigned GetEffectiveDelay();
	wxString GetEffectiveAudioGroup();

	unsigned GetEffectiveBitsPerSample();
	bool GetEffectiveCompress();
	unsigned GetEffectiveLoopLoad();
	unsigned GetEffectiveAttackLoad();
	unsigned GetEffectiveReleaseLoad();
	unsigned GetEffectiveChannels();

	virtual void AddChild(GOrguePipeConfigNode* node);
	virtual unsigned GetChildCount();
	virtual GOrguePipeConfigNode* GetChild(unsigned index);
};

#endif
