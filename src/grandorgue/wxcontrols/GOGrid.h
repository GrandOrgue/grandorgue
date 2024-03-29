/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGRID_H
#define GOGRID_H

#include <vector>

#include <wx/grid.h>

class GOAdditionalSizeKeeper;

class GOGrid : public wxGrid {
private:
  wxGridCellStringRenderer *p_NormalVisibleRenderer;
  wxGridCellStringRenderer *p_RightVisibleRenderer;
  std::vector<bool> m_AreColumnsRightVisible;

public:
  GOGrid(
    wxWindow *parent,
    wxWindowID id,
    const wxPoint &pos = wxDefaultPosition,
    const wxSize &size = wxDefaultSize,
    long style = wxWANTS_CHARS,
    const wxString &name = wxGridNameStr);
  virtual ~GOGrid();

  bool IsColumnRightVisible(unsigned colN) const;
  void SetColumnRightVisible(unsigned colN, bool isRightVisible);

  void ApplyColumnSizes(const GOAdditionalSizeKeeper &sizeKeeper);
  void CaptureColumnSizes(GOAdditionalSizeKeeper &sizeKeeper) const;

  virtual wxGridCellRenderer *GetDefaultRendererForCell(
    int row, int col) const override;
};

#endif /* GOGRID_H */
