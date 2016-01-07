/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2015 GrandOrgue contributors (see AUTHORS)
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

#ifndef GORGUESETTER_H
#define GORGUESETTER_H

#include "GOrgueControlChangedHandler.h"
#include "GOrgueElementCreator.h"
#include "GOrgueEnclosure.h"
#include "GOrgueLabel.h"
#include "GOrguePlaybackStateHandler.h"
#include "GOrgueSetterButtonCallback.h"

class GOrgueFrameGeneral;

typedef enum 
{
	SETTER_REGULAR,
	SETTER_SCOPE,
	SETTER_SCOPED
} SetterType;

class GOrgueSetter : private GOrgueSetterButtonCallback, private GOrguePlaybackStateHandler, private GOrgueControlChangedHandler,
	public GOrgueElementCreator
{
private:
	GrandOrgueFile* m_organfile;
	unsigned m_pos;
	unsigned m_bank;
	unsigned m_crescendopos;
	unsigned m_crescendobank;
	ptr_vector<GOrgueFrameGeneral> m_framegeneral;
	ptr_vector<GOrgueFrameGeneral> m_general;
	ptr_vector<GOrgueFrameGeneral> m_crescendo;
	GOrgueLabel m_PosDisplay;
	GOrgueLabel m_BankDisplay;
	GOrgueLabel m_CrescendoDisplay;
	GOrgueLabel m_TransposeDisplay;
	GOrgueLabel m_NameDisplay;
	GOrgueEnclosure m_swell;
	SetterType m_SetterType;

	void SetSetterType(SetterType type);
	void SetCrescendoType(unsigned no);
	void Crescendo(int pos, bool force = false);

	static const struct ElementListEntry m_element_types[];
	const struct ElementListEntry* GetButtonList();

	void SetterButtonChanged(GOrgueSetterButton* button);

	void ControlChanged(void* control);

	void AbortPlayback();
	void PreparePlayback();
	void StartPlayback();
	void PrepareRecording();

public:
	GOrgueSetter(GrandOrgueFile* organfile);
	virtual ~GOrgueSetter();

	void Load(GOrgueConfigReader& cfg);
	GOrgueEnclosure* GetEnclosure(const wxString& name, bool is_panel);
	GOrgueLabel* GetLabel(const wxString& name, bool is_panel);

	void Update();

	bool StoreInvisibleObjects();
	bool IsSetterActive();
	void ToggleSetter();
	void SetterActive(bool on);
	SetterType GetSetterType();

	void Next();
	void Prev();
	void Push();
	unsigned GetPosition();
	void UpdatePosition(int pos);
	void SetPosition(int pos, bool push = true);
	void ResetDisplay();
	void SetTranspose(int value);
	void UpdateTranspose();
	void UpdateModified(bool modified);
};

#endif
