/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOStopsWindow.h"

#include <wx/checkbox.h>
#include <wx/intl.h>
#include <wx/sizer.h>

#include "gui/wxcontrols/go_gui_utils.h"

#include "model/GOCoupler.h"
#include "model/GODivisionalCoupler.h"
#include "model/GOManual.h"
#include "model/GOOrganModel.h"
#include "model/GOStop.h"
#include "model/GOSwitch.h"
#include "model/GOTremulant.h"

constexpr unsigned MAX_ELEMENTS = 2000;

enum {
  ID_CHECKBOX = 200,
};

BEGIN_EVENT_TABLE(GOStopsWindow, wxFrame)
EVT_CHECKBOX(ID_CHECKBOX, GOStopsWindow::OnElementChanging)
EVT_SHOW(GOStopsWindow::OnShow)
END_EVENT_TABLE()

GOStopsWindow::GOStopsWindow(
  GODocumentBase *doc,
  wxWindow *parent,
  GODialogSizeSet &dialogSizes,
  GOOrganModel &model)
  : wxFrame(parent, wxID_ANY, _("Stops")),

    GOView(doc, this),
    r_SizeKeeper(dialogSizes.AssureSizeKeeperFor(wxT("Stops"), wxEmptyString)),
    r_model(model) {
  SetIcon(get_go_icon());
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
    mainSizer->Add(pSizer, 1, wxEXPAND | wxALL, 5);
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

      if (
        sizerIndex >= 0 && sizerIndex <= (int)globalSectionN && pName
        && e.control->IsControlledByUser()) {
        wxCheckBox *pCheckBox = new wxCheckBox(this, ID_CHECKBOX, *pName);

        pCheckBox->SetClientData(e.control);
        mp_checkboxesByControl[e.control] = pCheckBox;
        pCheckBox->SetValue(e.control->GetCombinationState());
        sizers[sizerIndex]->Add(pCheckBox, 1, wxEXPAND | wxALL, 5);
      }
    }
  }
  SetSizerAndFit(mainSizer);
}

void GOStopsWindow::OnShow(wxShowEvent &event) {
  if (event.IsShown()) {
    r_SizeKeeper.ApplySizeInfo(*this);
    r_model.RegisterControlChangedHandler(this);
  } else {
    r_model.UnRegisterControlChangedHandler(this);
    r_SizeKeeper.CaptureSizeInfo(*this);
  }
}

void GOStopsWindow::OnElementChanging(wxCommandEvent &event) {
  wxCheckBox *pCheck = static_cast<wxCheckBox *>(event.GetEventObject());
  GOCombinationElement *pE
    = static_cast<GOCombinationElement *>(pCheck->GetClientData());

  if (pE)
    pE->SetCombinationState(event.IsChecked(), wxEmptyString);
}

void GOStopsWindow::ControlChanged(GOControl *pControl) {
  auto pElement = dynamic_cast<GOCombinationElement *>(pControl);

  if (pElement) {
    auto it = mp_checkboxesByControl.find(pElement);

    if (it != mp_checkboxesByControl.end())
      it->second->SetValue(pElement->GetCombinationState());
  }
}

void GOStopsWindow::SyncState() { r_SizeKeeper.CaptureSizeInfo(*this); }
