/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOStopsWindow.h"

#include <wx/checkbox.h>
#include <wx/display.h>
#include <wx/intl.h>
#include <wx/scrolwin.h>
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
  GOSizeKeeper &sizeKeeper,
  GOOrganModel &model)
  : wxFrame(parent, wxID_ANY, _("Stops")),
    GOView(doc, this),
    r_SizeKeeper(sizeKeeper),
    r_model(model) {
  SetIcon(get_go_icon());
  wxScrolledWindow *sw = new wxScrolledWindow(this, wxID_ANY);

  wxBoxSizer *const mainSizer = new wxBoxSizer(wxHORIZONTAL);
  const unsigned nOdfManuals = model.GetODFManualCount();
  const unsigned globalSectionN = nOdfManuals;
  std::vector<wxStaticBoxSizer *> sizers;

  sizers.resize(globalSectionN + 1);
  for (unsigned i = model.GetFirstManualIndex(); i <= nOdfManuals; i++) {
    GOManual *const pManual = model.GetManual(i);
    wxStaticBoxSizer *const pSizer = new wxStaticBoxSizer(
      wxVERTICAL, sw, i < nOdfManuals ? pManual->GetName() : _("Globals"));

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

      if (
        sizerIndex >= 0 && sizerIndex <= (int)globalSectionN && pName
        && e.control->IsControlledByUser()) {
        wxCheckBox *pCheckBox = new wxCheckBox(sw, ID_CHECKBOX, *pName);

        pCheckBox->SetClientData(e.control);
        mp_checkboxesByControl[e.control] = pCheckBox;
        pCheckBox->SetValue(e.control->GetCombinationState());
        sizers[sizerIndex]->Add(pCheckBox, 0, wxEXPAND | wxALL, 5);
      }
    }
  }
  sw->SetScrollRate(5, 5);
  sw->SetSizerAndFit(mainSizer);

  // Set the optimal window size
  SetClientSize(sw->GetVirtualSize());

  // Lower the size if it does not fit the display.
  wxRect max = wxDisplay(wxDisplay::GetFromWindow(this)).GetClientArea();
  wxRect rect = GetRect();

  // Check if the window fits the current display
  if (!max.Contains(rect)) {
    // Otherwise, check and correct width and height, The scrollbar will appear
    if (rect.GetWidth() > max.GetWidth()) {
      rect.SetWidth(max.GetWidth());
      rect.SetX(max.GetX());
    }
    if (rect.GetHeight() > max.GetHeight()) {
      rect.SetHeight(max.GetHeight());
      rect.SetY(max.GetY());
    }
    SetSize(rect);
  }
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
