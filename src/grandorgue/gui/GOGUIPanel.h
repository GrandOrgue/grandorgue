/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUIPANEL_H
#define GOGUIPANEL_H

#include <wx/gdicmn.h>
#include <wx/string.h>

#include "ptrvector.h"

#include "primitives/GOBitmap.h"

#include "GOSaveableObject.h"

class GOGUIControl;
class GOGUIDisplayMetrics;
class GOGUILayoutEngine;
class GOGUIMouseState;
class GOGUIPanelWidget;
class GOConfigReader;
class GOConfigWriter;
class GOButtonControl;
class GODC;
class GOPanelView;
class GOOrganController;

#define GOBitmapPrefix "../GO:"

class GOGUIPanel : private GOSaveableObject {
private:
  void ReadSizeInfoFromCfg(GOConfigReader &cfg, bool isOpenByDefault);
  void LoadButton(
    GOConfigReader &cfg,
    GOButtonControl *pButtonControl,
    const wxString &groupPrefix,
    unsigned groupIndex,
    bool isPiston = false);
  void LoadManualButton(
    GOConfigReader &cfg,
    GOButtonControl *pButtonControl,
    const wxString &manualGroup,
    const wxString &groupPrefix,
    unsigned elementIndex,
    bool isPiston = false);

protected:
  GOOrganController *m_OrganController;
  GOGUIMouseState &m_MouseState;
  ptr_vector<GOGUIControl> m_controls;
  std::vector<GOBitmap> m_WoodImages;
  unsigned m_BackgroundControls;
  wxString m_Name;
  wxString m_GroupName;
  GOGUIDisplayMetrics *m_metrics;
  GOGUILayoutEngine *m_layout;
  GOPanelView *m_view;
  wxRect m_rect;
  int m_DisplayNum;
  bool m_IsMaximized;
  bool m_InitialOpenWindow;

  void LoadControl(GOGUIControl *control, GOConfigReader &cfg, wxString group);
  void LoadBackgroundControl(
    GOGUIControl *control, GOConfigReader &cfg, wxString group);
  void Save(GOConfigWriter &cfg);

  GOGUIControl *CreateGUIElement(GOConfigReader &cfg, wxString group);

  void SendMousePress(int x, int y, bool right, GOGUIMouseState &state);

public:
  GOGUIPanel(GOOrganController *organController);
  virtual ~GOGUIPanel();
  void Init(
    GOConfigReader &cfg,
    GOGUIDisplayMetrics *metrics,
    wxString name,
    wxString group,
    wxString group_name = wxT(""));
  void Load(GOConfigReader &cfg, wxString group);
  void Layout();

  void SetView(GOPanelView *view);

  GOOrganController *GetOrganFile();
  const wxString &GetGroup() { return m_group; }
  const wxString &GetName();
  const wxString &GetGroupName();
  void AddEvent(GOGUIControl *control);
  void AddControl(GOGUIControl *control);
  GOGUIDisplayMetrics *GetDisplayMetrics();
  GOGUILayoutEngine *GetLayoutEngine();
  void PrepareDraw(double scale, GOBitmap *background);
  void Draw(GODC &dc);
  const GOBitmap &GetWood(unsigned which);
  GOBitmap LoadBitmap(wxString filename, wxString maskname);
  void HandleKey(int key);
  void HandleMousePress(int x, int y, bool right);
  void HandleMouseRelease(bool right);
  void HandleMouseScroll(int x, int y, int amount);
  void Modified();

  unsigned GetWidth();
  unsigned GetHeight();
  bool InitialOpenWindow();

  wxRect GetWindowRect();
  void SetWindowRect(wxRect rect);
  int GetDisplayNum() const { return m_DisplayNum; }
  void SetDisplayNum(int displayNum) { m_DisplayNum = displayNum; }
  bool IsMaximized() const { return m_IsMaximized; }
  void SetMaximized(bool isMaximized) { m_IsMaximized = isMaximized; }
  void SetInitialOpenWindow(bool open);
};

#endif
