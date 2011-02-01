/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
 *
 * MyOrgan 1.0.6 Codebase - Copyright 2006 Milan Digital Audio LLC
 * MyOrgan is a Trademark of Milan Digital Audio LLC
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "GOrgueDrawStop.h"
#include "IniFileConfig.h"
#include "GrandOrgueFile.h"
#include "GrandOrgue.h"
#include "GrandOrgueFrame.h"
#include "GOrgueSound.h"
#include "OrganFile.h"
#include "SettingsDialog.h"
#include <wx/docview.h>
extern GrandOrgueFile* organfile;
extern GOrgueSound* g_sound;
extern const char* s_MIDIMessages[];


void GOrgueDrawstop::Load(IniFileConfig& cfg, const char* group)
{
  DispDrawstopRow		   = cfg.ReadInteger( group,"DispDrawstopRow",    1, 99 + organfile->m_DispExtraDrawstopRows);
  DispDrawstopCol		   = cfg.ReadInteger( group,"DispDrawstopCol",    1, DispDrawstopRow > 99 ? organfile->m_DispExtraDrawstopCols : organfile->m_DispDrawstopCols);
  DefaultToEngaged		   = cfg.ReadBoolean( group,"DefaultToEngaged");
  DisplayInInvertedState   = cfg.ReadBoolean( group,"DisplayInInvertedState");
  DispImageNum			   = cfg.ReadInteger( group,"DispImageNum",    1,    2);
  StopControlMIDIKeyNumber = cfg.ReadInteger( group,"StopControlMIDIKeyNumber",    0,  127, false);

  GOrgueControl::Load(cfg, group);
}

bool GOrgueDrawstop::Draw(int xx, int yy, wxDC* dc, wxDC* dc2)
{
  int x, y, i;
  if (!Displayed)
	return false;

  if (DispDrawstopRow > 99)
	{
	  x = organfile->m_JambTopX + (DispDrawstopCol - 1) * 78 + 6;
	  y = organfile->m_JambTopDrawstop + (DispDrawstopRow - 100) * 69 + 2;
	}
  else
	{
	  i = organfile->m_DispDrawstopCols >> 1;
	  if (DispDrawstopCol <= i)
		{
		  x = organfile->m_JambLeftX + (DispDrawstopCol - 1) * 78 + 6;
		  y = organfile->m_JambLeftRightY + (DispDrawstopRow - 1) * 69 + 32;
		}
	  else
		{
		  x = organfile->m_JambRightX + (DispDrawstopCol - 1 - i) * 78 + 6;
		  y = organfile->m_JambLeftRightY + (DispDrawstopRow - 1) * 69 + 32;
		}
	  if (organfile->m_DispPairDrawstopCols)
		x += (((DispDrawstopCol - 1) % i) >> 1) * 18;
	  if (DispDrawstopCol <= i)
		i = DispDrawstopCol;
	  else
		i = organfile->m_DispDrawstopCols - DispDrawstopCol + 1;
	  if (organfile->m_DispDrawstopColsOffset && (i & 1) ^ organfile->m_DispDrawstopOuterColOffsetUp)
		y += 35;
	}

  if (!dc)
	return !(xx < x || xx > x + 64 || yy < y || yy > y + 64 || (x + 32 - xx) * (x + 32 - xx) + (y + 32 - yy) * (y + 32 - yy) > 1024);

  wxRect rect(x, y + 1, 65, 65 - 1);
  wxBitmap bmp = organfile->m_images[((DispImageNum - 1) << 1) + (DisplayInInvertedState ^ DefaultToEngaged ? 1 : 0)];
  dc->DrawBitmap(bmp, x, y, true);
  dc->SetTextForeground(DispLabelColour);
  wxFont font = *wxNORMAL_FONT;
  font.SetFaceName(organfile->m_DispControlLabelFont);
  font.SetPointSize(DispLabelFontSize);
  dc->SetFont(font);
  dc->DrawLabel(Name, rect, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);

  if (dc2)
	dc2->Blit(x, y, 65, 65, dc, x, y);
  return false;
}

bool GOrgueDrawstop::Set(bool on)
{
  if (DefaultToEngaged == on)
	return on;
  DefaultToEngaged = on;
  wxCommandEvent event(wxEVT_DRAWSTOP, 0);
  event.SetClientData(this);
  ::wxGetApp().frame->AddPendingEvent(event);
  for (int i = 0; i < organfile->m_NumberOfReversiblePistons; i++)
	{
	  if (organfile->m_piston[i].drawstop == this)
	    {
		  organfile->m_piston[i].DispImageNum = (organfile->m_piston[i].DispImageNum & 1) | (on ^ DisplayInInvertedState ? 2 : 0);
		  wxCommandEvent event(wxEVT_PUSHBUTTON, 0);
		  event.SetClientData(organfile->m_piston + i);
		  ::wxGetApp().frame->AddPendingEvent(event);
	    }
	}
#ifdef __VFD__
  GOrgueLCD_WriteLineTwo(Name, 2000);
#endif
  return !on;
}

void GOrgueDrawstop::MIDI(void)
{
  MIDIListenDialog dlg(::wxGetApp().frame, _("Drawstop Trigger"), g_sound->i_midiEvents[14] | StopControlMIDIKeyNumber, 4);
  if (dlg.ShowModal() == wxID_OK)
	{
	  StopControlMIDIKeyNumber = dlg.GetEvent() & 0x7F;
	  ::wxGetApp().m_docManager->GetCurrentDocument()->Modify(true);
	}
}
