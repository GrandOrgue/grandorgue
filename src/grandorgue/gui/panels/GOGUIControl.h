/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUICONTROL_H
#define GOGUICONTROL_H

#include <wx/gdicmn.h>
#include <wx/string.h>

#include "control/GOControlChangedHandler.h"

#include "GOGUIPanel.h"
#include "GOSaveableObject.h"

class GOGUIDisplayMetrics;
class GOGUILayoutEngine;
class GOGUIMouseState;
class GOGUIPanel;
class GOBitmap;
class GOConfigReader;
class GOControl;
class GODC;

class GOGUIControl : private GOSaveableObject,
                     protected GOControlChangedHandler {
protected:
  GOGUIPanel *m_panel;
  GOGUIDisplayMetrics *m_metrics;
  GOGUILayoutEngine *m_layout;
  GOControl *p_control;
  wxRect m_BoundingRect;
  bool m_DrawPending;

  void Init(GOConfigReader &cfg, wxString group);
  void Save(GOConfigWriter &cfg) override;

  void ControlChanged(GOControl *control) override;

public:
  GOGUIControl(GOGUIPanel *panel, GOControl *pControl);
  virtual ~GOGUIControl();

  GODocumentBase *GetDocument() const {
    return m_panel ? m_panel->GetDocument() : nullptr;
  }

  virtual void Load(GOConfigReader &cfg, wxString group);
  virtual void Layout();

  virtual void PrepareDraw(double scale, GOBitmap *background);
  virtual void Draw(GODC &dc);
  virtual const wxRect &GetBoundingRect();
  virtual bool HandleMousePress(
    int x, int y, bool right, GOGUIMouseState &state);
  virtual bool HandleMouseScroll(int x, int y, int amount);
};

#endif
