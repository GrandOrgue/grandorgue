/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUIBUTTON_H
#define GOGUIBUTTON_H

#include <optional>

#include <wx/colour.h>

#include "primitives/GOBitmap.h"
#include "primitives/GOFont.h"

#include "GOGUILayoutEngine.h"
#include "GOGUIMidiControl.h"

class GOButtonControl;

class GOGUIButton : public GOGUIMidiControl {
protected:
  bool m_IsPiston;
  bool m_DispKeyLabelOnLeft;
  GOButtonControl *p_ButtonControl;
  wxRect m_MouseRect;
  unsigned m_Radius;
  GOBitmap m_OnBitmap;
  GOBitmap m_OffBitmap;
  unsigned m_FontSize;
  wxString m_FontName;
  GOFont m_Font;
  wxColor m_TextColor;
  wxString m_DispLabelText;
  wxRect m_TextRect;
  unsigned m_TextWidth;
  unsigned m_DispCol;
  unsigned m_DispRow;
  /** The grid of the cell (m_DispRow, m_DispCol) in which the button is placed
   */
  GOGUILayoutEngine::GridType m_GridType;
  unsigned m_TileOffsetX;
  unsigned m_TileOffsetY;

public:
  GOGUIButton(
    GOGUIPanel *panel, GOButtonControl *control, bool is_piston = false);

  /**
   * Initialises a button that is not defined in the ODF.
   * @param cfg the config reader
   * @param group the .cmb group of the button
   * @param x_pos the column of the cell in the grid
   * @param y_pos the row of the cell in the grid
   * @param dispImageNum the number of the set of the images of the button
   *   (piston%02d_*, drawstop%02d_*), as the ODF DispImageNum; 0 for the
   *   default one: 1, or 3 (piston) / 4 (drawstop) if the button is read-only
   * @param dispLabelText the label drawn on the button; if not given, the name
   *   of the button control is used; an empty string draws no text. (Load()
   *   reads it from the ODF DispLabelText)
   * @param gridType the grid of the cell; if not given, GRID_PUSHBUTTON for a
   *   piston and GRID_DRAWSTOP otherwise
   */
  void Init(
    GOConfigReader &cfg,
    wxString group,
    unsigned x_pos,
    unsigned y_pos,
    unsigned dispImageNum = 0,
    const std::optional<wxString> &dispLabelText = std::nullopt,
    std::optional<GOGUILayoutEngine::GridType> gridType = std::nullopt);
  void Load(GOConfigReader &cfg, wxString group);
  void Layout();

  bool HandleMousePress(int x, int y, bool right, GOGUIMouseState &state);
  void PrepareDraw(double scale, GOBitmap *background);
  void Draw(GODC &dc);
};

#endif
