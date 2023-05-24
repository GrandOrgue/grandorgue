/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGUIManual.h"

#include <wx/intl.h>
#include <wx/log.h>

#include "config/GOConfigReader.h"
#include "model/GOManual.h"
#include "primitives/GODC.h"

#include "GOGUIDisplayMetrics.h"
#include "GOGUILayoutEngine.h"
#include "GOGUIMouseState.h"
#include "GOGUIPanel.h"

GOGUIManual::GOGUIManual(
  GOGUIPanel *panel, GOManual *manual, unsigned manual_number)
  : GOGUIControl(panel, manual),
    m_manual(manual),
    m_ManualNumber(manual_number),
    m_Keys() {
  m_layout->RegisterManual(this);
}

bool GOGUIManual::IsSharp(unsigned key) { return m_Keys[key].IsSharp; }

unsigned GOGUIManual::GetKeyCount() { return m_Keys.size(); }

void GOGUIManual::Init(GOConfigReader &cfg, wxString group) {
  const wxChar *keyNames[12] = {
    wxT("C"),
    wxT("Cis"),
    wxT("D"),
    wxT("Dis"),
    wxT("E"),
    wxT("F"),
    wxT("Fis"),
    wxT("G"),
    wxT("Gis"),
    wxT("A"),
    wxT("Ais"),
    wxT("B")};

  GOGUIControl::Init(cfg, group);
  wxString type = m_ManualNumber ? wxT("Manual") : wxT("Pedal");
  unsigned first_midi_note = m_manual->GetFirstAccessibleKeyMIDINoteNumber();

  unsigned x = 0, y = 0;
  int width = 0;
  int height = 1;

  m_Keys.resize(m_manual->GetNumberOfAccessibleKeys());
  for (unsigned i = 0; i < m_Keys.size(); i++) {
    unsigned key_nb = i + first_midi_note;
    m_Keys[i].MidiNumber = key_nb;
    m_Keys[i].IsSharp = (((key_nb % 12) < 5 && !(key_nb & 1))
                         || ((key_nb % 12) >= 5 && (key_nb & 1)))
      ? false
      : true;
  }

  for (unsigned i = 0; i < m_Keys.size(); i++) {
    unsigned key_nb = i + first_midi_note;
    wxString off_mask_file, on_mask_file;
    wxString on_file, off_file;
    wxString bmp_type;
    unsigned key_width, key_yoffset;
    int key_offset;
    wxString base = keyNames[key_nb % 12];
    if (!i)
      base = wxT("First") + base;
    else if (i + 1 == m_Keys.size())
      base = wxT("Last") + base;
    bool prev_is_sharp = i > 0 ? m_Keys[i - 1].IsSharp : false;
    bool next_is_sharp = i + 1 < m_Keys.size() ? m_Keys[i + 1].IsSharp : false;

    if (!m_ManualNumber) {
      bmp_type = m_Keys[i].IsSharp ? wxT("Sharp") : wxT("Natural");
    } else {
      if (m_Keys[i].IsSharp)
        bmp_type = wxT("Sharp");
      else if (!prev_is_sharp && next_is_sharp)
        bmp_type = wxT("C");
      else if (prev_is_sharp && next_is_sharp)
        bmp_type = wxT("D");
      else if (prev_is_sharp && !next_is_sharp)
        bmp_type = wxT("E");
      else
        bmp_type = wxT("Natural");
    }
    off_file = wxT(GOBitmapPrefix "") + type + wxT("01Off_") + bmp_type;
    on_file = wxT(GOBitmapPrefix "") + type + wxT("01On_") + bmp_type;

    on_mask_file = wxEmptyString;
    off_mask_file = on_mask_file;

    m_Keys[i].OnBitmap = m_panel->LoadBitmap(on_file, on_mask_file);
    m_Keys[i].OffBitmap = m_panel->LoadBitmap(off_file, off_mask_file);

    if (
      m_Keys[i].OnBitmap.GetWidth() != m_Keys[i].OffBitmap.GetWidth()
      || m_Keys[i].OnBitmap.GetHeight() != m_Keys[i].OffBitmap.GetHeight())
      throw wxString::Format(
        _("bitmap size does not match for '%s'"), group.c_str());

    key_width = m_Keys[i].OnBitmap.GetWidth();
    key_offset = 0;
    key_yoffset = 0;
    if (m_Keys[i].IsSharp && m_ManualNumber) {
      key_width = 0;
      key_offset = -((int)m_Keys[i].OnBitmap.GetWidth()) / 2;
    } else if (!m_ManualNumber && !next_is_sharp && !m_Keys[i].IsSharp) {
      key_width *= 2;
    }

    m_Keys[i].Rect = wxRect(
      x + key_offset,
      y + key_yoffset,
      m_Keys[i].OnBitmap.GetWidth(),
      m_Keys[i].OnBitmap.GetHeight());
    if (x + key_offset < 0)
      wxLogWarning(
        _("Manual key %d outside of the bounding box"), m_Keys[i].MidiNumber);

    unsigned mouse_x = 0;
    unsigned mouse_y = 0;
    unsigned mouse_w = m_Keys[i].Rect.GetWidth() - mouse_x;
    unsigned mouse_h = m_Keys[i].Rect.GetHeight() - mouse_y;
    m_Keys[i].MouseRect = wxRect(
      m_Keys[i].Rect.GetX() + mouse_x,
      m_Keys[i].Rect.GetY() + mouse_y,
      mouse_w,
      mouse_h);

    if (height < m_Keys[i].Rect.GetBottom())
      height = m_Keys[i].Rect.GetBottom();
    if (width < m_Keys[i].Rect.GetRight())
      width = m_Keys[i].Rect.GetRight();
    x += key_width;
  }

  m_BoundingRect = wxRect(0, 0, width + 1, height + 1);
  m_LeftTop = wxPoint(-1, -1);
}

void GOGUIManual::Load(GOConfigReader &cfg, wxString group) {
  const wxChar *keyNames[12] = {
    wxT("C"),
    wxT("Cis"),
    wxT("D"),
    wxT("Dis"),
    wxT("E"),
    wxT("F"),
    wxT("Fis"),
    wxT("G"),
    wxT("Gis"),
    wxT("A"),
    wxT("Ais"),
    wxT("B")};

  GOGUIControl::Load(cfg, group);
  bool color_inverted = cfg.ReadBoolean(
    ODFSetting, group, wxT("DispKeyColourInverted"), false, false);
  bool color_wooden = cfg.ReadBoolean(
    ODFSetting, group, wxT("DispKeyColourWooden"), false, false);
  wxString type = m_ManualNumber ? wxT("Manual") : wxT("Pedal");
  if (color_inverted)
    type += wxT("Inverted");
  if (color_wooden && m_ManualNumber)
    type += wxT("Wood");
  unsigned first_midi_note = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("DisplayFirstNote"),
    0,
    127,
    false,
    m_manual->GetFirstAccessibleKeyMIDINoteNumber());

  int x = 0;
  int y = 0;
  int top = 0;
  int left = 0;
  int right = 0;
  int bottom = 1;

  m_Keys.resize(cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("DisplayKeys"),
    1,
    m_manual->GetNumberOfAccessibleKeys(),
    false,
    m_manual->GetNumberOfAccessibleKeys()));
  for (unsigned i = 0; i < m_Keys.size(); i++) {
    unsigned key_nb = i + first_midi_note;
    m_Keys[i].MidiNumber = cfg.ReadInteger(
      ODFSetting,
      group,
      wxString::Format(wxT("DisplayKey%03d"), i + 1),
      0,
      127,
      false,
      key_nb);
    key_nb = cfg.ReadInteger(
      ODFSetting,
      group,
      wxString::Format(wxT("DisplayKey%03dNote"), i + 1),
      0,
      127,
      false,
      key_nb);
    m_Keys[i].IsSharp = (((key_nb % 12) < 5 && !(key_nb & 1))
                         || ((key_nb % 12) >= 5 && (key_nb & 1)))
      ? false
      : true;
  }

  for (unsigned i = 0; i < m_Keys.size(); i++) {
    unsigned key_nb = i + first_midi_note;
    key_nb = cfg.ReadInteger(
      ODFSetting,
      group,
      wxString::Format(wxT("DisplayKey%03dNote"), i + 1),
      0,
      127,
      false,
      key_nb);
    wxString off_mask_file, on_mask_file;
    wxString on_file, off_file;
    wxString bmp_type;
    unsigned key_width;
    int key_yoffset;
    int key_offset;
    wxString base = keyNames[key_nb % 12];
    if (!i)
      base = wxT("First") + base;
    else if (i + 1 == m_Keys.size())
      base = wxT("Last") + base;
    bool prev_is_sharp = i > 0 ? m_Keys[i - 1].IsSharp : false;
    bool next_is_sharp = i + 1 < m_Keys.size() ? m_Keys[i + 1].IsSharp : false;

    if (!m_ManualNumber) {
      bmp_type = m_Keys[i].IsSharp ? wxT("Sharp") : wxT("Natural");
    } else {
      if (m_Keys[i].IsSharp)
        bmp_type = wxT("Sharp");
      else if (!prev_is_sharp && next_is_sharp)
        bmp_type = wxT("C");
      else if (prev_is_sharp && next_is_sharp)
        bmp_type = wxT("D");
      else if (prev_is_sharp && !next_is_sharp)
        bmp_type = wxT("E");
      else
        bmp_type = wxT("Natural");
    }

    int DispImageNum
      = cfg.ReadInteger(ODFSetting, group, wxT("DispImageNum"), 1, 2, false, 1);

    off_file = wxT(GOBitmapPrefix) + type
      + wxString::Format(wxT("%02dOff_"), DispImageNum) + bmp_type;
    on_file = wxT(GOBitmapPrefix) + type
      + wxString::Format(wxT("%02dOn_"), DispImageNum) + bmp_type;

    on_file = cfg.ReadStringTrim(
      ODFSetting, group, wxT("ImageOn_") + base, false, on_file);
    off_file = cfg.ReadStringTrim(
      ODFSetting, group, wxT("ImageOff_") + base, false, off_file);
    on_mask_file = cfg.ReadStringTrim(
      ODFSetting, group, wxT("MaskOn_") + base, false, wxEmptyString);
    off_mask_file = cfg.ReadStringTrim(
      ODFSetting, group, wxT("MaskOff_") + base, false, on_mask_file);

    on_file = cfg.ReadStringTrim(
      ODFSetting,
      group,
      wxString::Format(wxT("Key%03dImageOn"), i + 1),
      false,
      on_file);
    off_file = cfg.ReadStringTrim(
      ODFSetting,
      group,
      wxString::Format(wxT("Key%03dImageOff"), i + 1),
      false,
      off_file);
    on_mask_file = cfg.ReadStringTrim(
      ODFSetting,
      group,
      wxString::Format(wxT("Key%03dMaskOn"), i + 1),
      false,
      on_mask_file);
    off_mask_file = cfg.ReadStringTrim(
      ODFSetting,
      group,
      wxString::Format(wxT("Key%03dMaskOff"), i + 1),
      false,
      off_mask_file);

    m_Keys[i].OnBitmap = m_panel->LoadBitmap(on_file, on_mask_file);
    m_Keys[i].OffBitmap = m_panel->LoadBitmap(off_file, off_mask_file);

    if (
      m_Keys[i].OnBitmap.GetWidth() != m_Keys[i].OffBitmap.GetWidth()
      || m_Keys[i].OnBitmap.GetHeight() != m_Keys[i].OffBitmap.GetHeight())
      throw wxString::Format(
        _("bitmap size does not match for '%s'"), group.c_str());

    key_width = m_Keys[i].OnBitmap.GetWidth();
    key_offset = 0;
    key_yoffset = 0;
    if (m_Keys[i].IsSharp && m_ManualNumber) {
      key_width = 0;
      key_offset = -((int)m_Keys[i].OnBitmap.GetWidth()) / 2;
    } else if (!m_ManualNumber && !next_is_sharp && !m_Keys[i].IsSharp) {
      key_width *= 2;
    }

    key_width = cfg.ReadInteger(
      ODFSetting, group, wxT("Width_") + base, 0, 500, false, key_width);
    key_offset = cfg.ReadInteger(
      ODFSetting, group, wxT("Offset_") + base, -500, 500, false, key_offset);
    key_yoffset = cfg.ReadInteger(
      ODFSetting, group, wxT("YOffset_") + base, -500, 500, false, key_yoffset);

    key_width = cfg.ReadInteger(
      ODFSetting,
      group,
      wxString::Format(wxT("Key%03dWidth"), i + 1),
      0,
      500,
      false,
      key_width);
    key_offset = cfg.ReadInteger(
      ODFSetting,
      group,
      wxString::Format(wxT("Key%03dOffset"), i + 1),
      -500,
      500,
      false,
      key_offset);
    key_yoffset = cfg.ReadInteger(
      ODFSetting,
      group,
      wxString::Format(wxT("Key%03dYOffset"), i + 1),
      -500,
      500,
      false,
      key_yoffset);

    wxRect &keyRect = m_Keys[i].Rect;

    keyRect = wxRect(
      x + key_offset,
      y + key_yoffset,
      m_Keys[i].OnBitmap.GetWidth(),
      m_Keys[i].OnBitmap.GetHeight());

    unsigned mouse_x = cfg.ReadInteger(
      ODFSetting,
      group,
      wxString::Format(wxT("Key%03dMouseRectLeft"), i + 1),
      0,
      keyRect.GetWidth() - 1,
      false,
      0);
    unsigned mouse_y = cfg.ReadInteger(
      ODFSetting,
      group,
      wxString::Format(wxT("Key%03dMouseRectTop"), i + 1),
      0,
      keyRect.GetHeight() - 1,
      false,
      0);
    unsigned mouse_w = cfg.ReadInteger(
      ODFSetting,
      group,
      wxString::Format(wxT("Key%03dMouseRectWidth"), i + 1),
      1,
      keyRect.GetWidth() - mouse_x,
      false,
      keyRect.GetWidth() - mouse_x);
    unsigned mouse_h = cfg.ReadInteger(
      ODFSetting,
      group,
      wxString::Format(wxT("Key%03dMouseRectHeight"), i + 1),
      1,
      keyRect.GetHeight() - mouse_y,
      false,
      keyRect.GetHeight() - mouse_y);
    m_Keys[i].MouseRect = wxRect(
      keyRect.GetX() + mouse_x, keyRect.GetY() + mouse_y, mouse_w, mouse_h);

    if (top > keyRect.GetTop())
      top = keyRect.GetTop();
    if (left > keyRect.GetLeft())
      left = keyRect.GetLeft();
    if (bottom < keyRect.GetBottom())
      bottom = keyRect.GetBottom();
    if (right < keyRect.GetRight())
      right = keyRect.GetRight();
    x += key_width;
  }

  m_LeftTop = wxPoint(
    cfg.ReadInteger(
      ODFSetting,
      group,
      wxT("PositionX"),
      0,
      m_metrics->GetScreenWidth(),
      false,
      -1),
    cfg.ReadInteger(
      ODFSetting,
      group,
      wxT("PositionY"),
      0,
      m_metrics->GetScreenHeight(),
      false,
      -1));
  m_BoundingRect = wxRect(left, top, right - left + 1, bottom - top + 1);
}

void GOGUIManual::Layout() {
  const GOGUILayoutEngine::MANUAL_RENDER_INFO &mri
    = m_layout->GetManualRenderInfo(m_ManualNumber);

  // Get the top left point
  if (m_LeftTop.x == -1)
    m_LeftTop.x = mri.x + 1;
  if (m_LeftTop.y == -1)
    m_LeftTop.y = mri.keys_y;

  // Move all rectangles to the top left point
  m_BoundingRect.Offset(m_LeftTop);
  for (unsigned i = 0; i < m_Keys.size(); i++) {
    m_Keys[i].Rect.Offset(m_LeftTop);
    m_Keys[i].MouseRect.Offset(m_LeftTop);
  }
}

void GOGUIManual::PrepareDraw(double scale, GOBitmap *background) {
  for (unsigned i = 0; i < m_Keys.size(); i++) {
    m_Keys[i].OnBitmap.PrepareBitmap(scale, m_Keys[i].Rect, background);
    m_Keys[i].OffBitmap.PrepareBitmap(scale, m_Keys[i].Rect, background);
  }
}

void GOGUIManual::Draw(GODC &dc) {
  for (unsigned i = 0; i < m_Keys.size(); i++) {
    GOBitmap &bitmap = m_manual->IsKeyDown(m_Keys[i].MidiNumber)
      ? m_Keys[i].OnBitmap
      : m_Keys[i].OffBitmap;
    dc.DrawBitmap(bitmap, m_Keys[i].Rect);
  }
  GOGUIControl::Draw(dc);
}

bool GOGUIManual::HandleMousePress(
  int x, int y, bool right, GOGUIMouseState &state) {
  if (!m_BoundingRect.Contains(x, y))
    return false;

  if (right) {
    m_manual->ShowConfigDialog();
    return true;
  } else {
    for (unsigned i = 0; i < m_Keys.size(); i++) {
      if (m_Keys[i].MouseRect.Contains(x, y)) {
        if (
          i + 1 < m_Keys.size() && m_Keys[i + 1].IsSharp
          && m_Keys[i + 1].MouseRect.Contains(x, y))
          continue;
        if (state.GetControl() == this && state.GetIndex() == i)
          return true;
        state.SetControl(this);
        state.SetIndex(i);

        m_manual->Set(
          m_Keys[i].MidiNumber,
          m_manual->IsKeyDown(m_Keys[i].MidiNumber) ? 0 : 0x7f);
        return true;
      }
    }
    return false;
  }
}
