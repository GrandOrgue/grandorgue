/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOStopsDialog.h"

#include <wx/checkbox.h>
#include <wx/intl.h>
#include <wx/sizer.h>

#include "model/GOCoupler.h"
#include "model/GODivisionalCoupler.h"
#include "model/GOManual.h"
#include "model/GOOrganModel.h"
#include "model/GOStop.h"
#include "model/GOSwitch.h"
#include "model/GOTremulant.h"

enum {
  ID_FIRST = 200,
};

GOStopsDialog::GOStopsDialog(
  GODocumentBase *doc,
  wxWindow *parent,
  GODialogSizeSet &dialogSizes,
  GOOrganModel &model)
  : GOSimpleDialog(
    parent,
    wxT("Stops"),
    _("Stops"),
    dialogSizes,
    wxEmptyString,
    0,
    wxCLOSE | wxHELP),
    GOView(doc, this),
    r_model(model) {
  wxBoxSizer *const mainSizer = new wxBoxSizer(wxHORIZONTAL);
  const unsigned nOdfManuals = model.GetODFManualCount();
  const unsigned globalSectionN = nOdfManuals;
  std::vector<wxStaticBoxSizer *> sizers;

  sizers.resize(globalSectionN + 1);
  for (unsigned i = model.GetFirstManualIndex(); i <= nOdfManuals; i++) {
    GOManual *const pManual = model.GetManual(i);
    wxStaticBoxSizer *const pSizer = new wxStaticBoxSizer(
      wxVERTICAL, this, i < nOdfManuals ? pManual->GetName() : _("Globals"));

    sizers[i] = pSizer;
    mainSizer->Add(pSizer, 0, wxEXPAND | wxALL, 5);
  }

  auto &elements = model.GetGeneralTemplate().GetElements();

  for (unsigned l = elements.size(), i = 0; i < l; i++) {
    auto &e = elements[i];

    if (e.store_unconditional) {
      int sizerIndex = e.manual;
      const int objectIndex = e.index - 1;
      const wxString *pName = nullptr;

      switch (e.type) {
      case GOCombinationDefinition::COMBINATION_STOP: {
        GOManual &manual = *model.GetManual(e.manual);

        pName = &manual.GetStop(objectIndex)->GetName();
      } break;

      case GOCombinationDefinition::COMBINATION_COUPLER: {
        GOManual &manual = *model.GetManual(e.manual);
        GOCoupler *pCoupler = manual.GetCoupler(objectIndex);

        if (!pCoupler->IsVirtual())
          pName = &manual.GetCoupler(objectIndex)->GetName();
      } break;

      case GOCombinationDefinition::COMBINATION_TREMULANT: {
        sizerIndex = globalSectionN;
        pName = &model.GetTremulant(objectIndex)->GetName();
      } break;

      case GOCombinationDefinition::COMBINATION_SWITCH:
        if (e.manual >= 0) { // manual switch
          GOManual &manual = *model.GetManual(e.manual);

          pName = &manual.GetSwitch(objectIndex)->GetName();
        } else { // global switch
          sizerIndex = globalSectionN;
          pName = &model.GetSwitch(objectIndex)->GetName();
        }
        break;

      case GOCombinationDefinition::COMBINATION_DIVISIONALCOUPLER: {
        sizerIndex = globalSectionN;
        pName = &model.GetDivisionalCoupler(objectIndex)->GetName();
      } break;
      }

      if (sizerIndex >= 0 && sizerIndex <= (int)globalSectionN && pName) {
        wxCheckBox *pCheckBox = new wxCheckBox(this, ID_FIRST + i, *pName);

        printf(
          "GOStopsDialog: %u %s: %p\n",
          sizerIndex,
          pName->c_str().AsChar(),
          e.control);

        mp_checkboxesByControl[e.control] = pCheckBox;
        pCheckBox->SetValue(e.control->GetCombinationState());
        sizers[sizerIndex]->Add(pCheckBox, 0, wxEXPAND | wxALL, 5);
      }
    }
  }

  LayoutWithInnerSizer(mainSizer);
  r_model.RegisterControlChangedHandler(this);
}

GOStopsDialog::~GOStopsDialog() {
  r_model.UnRegisterControlChangedHandler(this);
}

void GOStopsDialog::ControlChanged(GOControl *pControl) {
  auto pElement = dynamic_cast<GOCombinationElement *>(pControl);

  if (pElement) {
    auto it = mp_checkboxesByControl.find(pElement);

    if (it != mp_checkboxesByControl.end())
      it->second->SetValue(pElement->GetCombinationState());
  }
}
