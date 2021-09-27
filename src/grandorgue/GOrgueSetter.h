/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUESETTER_H
#define GORGUESETTER_H

#include "GOrgueControlChangedHandler.h"
#include "GOrgueElementCreator.h"
#include "GOrgueEnclosure.h"
#include "GOrgueLabel.h"
#include "GOrguePlaybackStateHandler.h"

class GOrgueFrameGeneral;

typedef enum 
{
	SETTER_REGULAR,
	SETTER_SCOPE,
	SETTER_SCOPED
} SetterType;

class GOrgueSetter : private GOrguePlaybackStateHandler, private GOrgueControlChangedHandler, 
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

	void ButtonChanged(int id);

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
