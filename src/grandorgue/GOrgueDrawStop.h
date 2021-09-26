/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEDRAWSTOP_H
#define GORGUEDRAWSTOP_H

#include "GOrgueButton.h"
#include "GOrgueCombinationElement.h"
#include <wx/string.h>

class GOrgueDrawstop : public GOrgueButton, public GOrgueCombinationElement
{
 public:
	typedef enum { FUNCTION_INPUT, FUNCTION_AND, FUNCTION_OR, FUNCTION_NOT, FUNCTION_XOR, FUNCTION_NAND, FUNCTION_NOR } GOrgueFunctionType;
private:
	static const struct IniFileEnumEntry m_function_types[];
	GOrgueFunctionType m_Type;
	int m_GCState;
	bool m_ActiveState;
	bool m_CombinationState;
	std::vector<GOrgueDrawstop*> m_ControlledDrawstops;
	std::vector<GOrgueDrawstop*> m_ControllingDrawstops;

protected:
	bool m_StoreDivisional;
	bool m_StoreGeneral;

	void SetState(bool on);
	virtual void ChangeState(bool on) = 0;
	virtual void SetupCombinationState() = 0;

	void Save(GOrgueConfigWriter& cfg);

	void PreparePlayback();

public:
	GOrgueDrawstop(GrandOrgueFile* organfile);
	void Init(GOrgueConfigReader& cfg, wxString group, wxString name);
	void Load(GOrgueConfigReader& cfg, wxString group);
	void RegisterControlled(GOrgueDrawstop* sw);
	virtual void Set(bool on);
	virtual void Update();
	void Reset();
	void SetCombination(bool on);

	bool IsActive() const;
	bool GetCombinationState() const;
	bool GetStoreDivisional() const;
	bool GetStoreGeneral() const;
};

#endif
