/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUILABEL_H
#define GOGUILABEL_H

#include <optional>

#include <wx/colour.h>

#include "primitives/GOFont.h"

#include "GOGUILayoutEngine.h"
#include "GOGUIMidiControl.h"

class GOBitmap;
class GOLabelControl;

class GOGUILabel : public GOGUIMidiControl {
private:
  int m_DispXpos;
  int m_DispYpos;
  GOLabelControl *m_Label;
  GOBitmap *m_PBackgroundBitmap; // if nullptr then the label is transparent
  GOFont m_Font;
  wxString m_Text;
  wxColour m_TextColor;
  wxRect m_TextRect;
  unsigned m_TextWidth;
  /** The grid in which the label is placed; empty if it is placed in pixels */
  std::optional<GOGUILayoutEngine::GridType> m_GridType;
  /** The column of the cell of the label in m_GridType, from 1 */
  unsigned m_GridColN;
  /** The row of the cell of the label in m_GridType */
  unsigned m_GridRowN;
  unsigned m_TileOffsetX;
  unsigned m_TileOffsetY;

  void DestroyBackgroundBitmap();
  void InitBackgroundBitmap(
    unsigned w,
    unsigned h,
    wxString imageFileName,
    unsigned imageNum,
    const wxString &imageMaskFileneme);
  void InitFont(const wxString &fontName, unsigned fontSize);

public:
  GOGUILabel(GOGUIPanel *panel, GOLabelControl *label);
  ~GOGUILabel() override { DestroyBackgroundBitmap(); }

  unsigned GetFontSize() const { return m_Font.GetPoints(); }
  void SetFontSize(unsigned fontSize) { m_Font.SetPoints(fontSize); }
  const wxString &GetFontName() const { return m_Font.GetName(); }
  void SetFontName(const wxString &fontName) { m_Font.SetName(fontName); }
  const wxColour &GetFontColour() { return m_TextColor; }
  void SetFontColour(const wxColour &colour) { m_TextColor = colour; }

  /**
   * Initialises a label that is not defined in the ODF.
   * @param cfg the config reader
   * @param group the .cmb group of the label
   * @param x_pos if gridType is not given: the X of the label in pixels;
   *   otherwise the column of the cell in the grid, from 1
   * @param y_pos if gridType is not given: the Y of the label in pixels;
   *   otherwise the row of the cell in the grid
   * @param name the text of the label
   * @param dispImageNum the number of the picture of the label (label%02d)
   * @param w the width of the label; 0 takes it from the picture. A bigger
   *   width crops the picture
   * @param h the height of the label; 0 takes it from the picture. A bigger
   *   height tiles the picture
   * @param gridType the grid in which the label is placed. In an enclosure
   *   cell the label is centred; in a drawstop or pushbutton cell it is placed
   *   as an item of the standard cell size, as
   *   GOGUILayoutEngine::GetBlitPosition() does, so a label of another size is
   *   not centred there. If not given, x_pos and y_pos are pixels
   */
  void Init(
    GOConfigReader &cfg,
    wxString group,
    unsigned x_pos = 0,
    unsigned y_pos = 0,
    wxString name = wxT(""),
    unsigned dispImageNum = 1,
    unsigned w = 0,
    unsigned h = 0,
    std::optional<GOGUILayoutEngine::GridType> gridType = std::nullopt);
  void Load(GOConfigReader &cfg, wxString group) override;
  void Layout() override;

  bool HandleMousePress(
    int x, int y, bool right, GOGUIMouseState &state) override;
  void PrepareDraw(double scale, GOBitmap *background) override;
  void Draw(GODC &dc) override;
};

#endif
