/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSETTINGSORGANS_H
#define GOSETTINGSORGANS_H

#include <unordered_map>
#include <unordered_set>

#include <wx/hashset.h>
#include <wx/panel.h>

#include "ptrvector.h"

class GOMidi;
class wxButton;
class wxListEvent;
class wxListView;
class wxTextCtrl;

class GOArchiveFile;
class GOConfig;
class GOOrgan;

class GOSettingsOrgans : public wxPanel {
public:
  enum {
    ID_ORGANS = 200,
    ID_ORGAN_DEL,
    ID_ORGAN_DOWN,
    ID_ORGAN_UP,
    ID_ORGAN_TOP,
    ID_ORGAN_MIDI,
    ID_ORGAN_RELOCATE,
    ID_DEL_CACHE,
    ID_DEL_PRESET
  };

private:
  struct PackageSlot {
    GOArchiveFile *p_OrigPkg;
    GOArchiveFile *p_CurrentPkg;
    bool is_present;

    PackageSlot(GOArchiveFile *pPkg)
      : p_OrigPkg(pPkg), p_CurrentPkg(pPkg), is_present(true) {}
  };
  using PackageSlotSet = std::unordered_set<const PackageSlot *>;
  struct OrganSlot {
    GOOrgan *p_OrigOrgan;
    GOOrgan *p_CurrentOrgan;
    bool is_packaged;
    wxString m_CurrentPath;
    bool is_AnyCacheExisting;
    bool is_AnyPresetExisting;
    PackageSlot *p_PackageSlot;
    bool is_present;
  };
  using OrganSlotToLongMap
    = std::unordered_map<const GOSettingsOrgans::OrganSlot *, long>;
  struct VisibleOrganRec {
    const OrganSlot *p_OrganSlot;
    bool is_selected;
    bool is_focused;
  };
  using VisibleOrganRecs = std::vector<VisibleOrganRec>;

  GOConfig &m_config;
  GOMidi &m_midi;
  ptr_vector<GOOrgan> &m_OrigOrganList;
  ptr_vector<GOArchiveFile> &m_OrigPackageList;

  std::vector<PackageSlot> m_PackageSlots;
  std::unordered_map<const wxString, PackageSlot *, wxStringHash, wxStringEqual>
    m_PackageSlotByPath;
  std::vector<OrganSlot> m_OrganSlots;
  std::unordered_map<const wxString, OrganSlot *, wxStringHash, wxStringEqual>
    m_OrganSlotByPath;

  wxListView *m_Organs;
  wxTextCtrl *m_Builder;
  wxTextCtrl *m_Recording;
  wxTextCtrl *m_OrganHash;
  wxTextCtrl *m_PackageId;
  wxTextCtrl *m_PackageName;
  wxTextCtrl *m_PathInPackage;
  wxTextCtrl *m_PackageHash;
  wxTextCtrl *m_PackageInfo;

  wxButton *m_OrganDown;
  wxButton *m_OrganUp;
  wxButton *m_OrganTop;
  wxButton *m_OrganDel;
  wxButton *m_OrganMidi;
  wxButton *m_OrganRelocate;
  wxButton *m_DelCache;
  wxButton *m_DelPreset;

  PackageSlotSet GetUsedPackages(
    std::unordered_set<long> itemsToExtract = std::unordered_set<long>());
  void RefreshFocused();
  void RefreshButtons();
  VisibleOrganRecs GetCurrentOrganRecs();

  static int wxCALLBACK
  organOrdCompareCallback(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData);

  void ReorderOrgans(const VisibleOrganRecs &newSortedRecs);
  void DelSelectedOrgans();
  void ReplaceOrganPath(const long index, const wxString &newPath);

  void OnCharHook(wxKeyEvent &ev);

  void OnOrganFocused(wxListEvent &event);
  void OnOrganSelected(wxListEvent &event);
  void OnOrganUp(wxCommandEvent &event);
  void OnOrganDown(wxCommandEvent &event);
  void OnOrganTop(wxCommandEvent &event);
  void OnOrganDel(wxCommandEvent &event) { DelSelectedOrgans(); }
  void OnOrganMidi(wxCommandEvent &event);
  void OnOrganRelocate(wxCommandEvent &event);
  void OnDelCache(wxCommandEvent &event);
  void OnDelPreset(wxCommandEvent &event);

public:
  GOSettingsOrgans(GOConfig &settings, GOMidi &midi, wxWindow *parent);

  virtual bool TransferDataToWindow() override;
  virtual bool TransferDataFromWindow() override;

  DECLARE_EVENT_TABLE()
};

#endif
