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

#include "OrganPanel.h"
#include <wx/docview.h>
#include <wx/event.h>
#include "GOrgueDrawStop.h"
#include "GrandOrgueFile.h"
#include <algorithm>
#include <wx/mstream.h>
#include "GOrgueDivisional.h"
#include "GOrgueSound.h"
#include "GrandOrgue.h"
#include "GrandOrgueID.h"
#include "GrandOrgueFrame.h"
#include "OrganDocument.h"
#include "GOrgueMeter.h"
#include "KeyConvert.h"

BEGIN_EVENT_TABLE(OrganPanel, wxPanel)
EVT_ERASE_BACKGROUND(OrganPanel::OnErase)
EVT_LEFT_DOWN(OrganPanel::OnMouseLeftDown)
EVT_LEFT_DCLICK(OrganPanel::OnMouseLeftDown)
EVT_RIGHT_DOWN(OrganPanel::OnMouseRightDown)
EVT_RIGHT_DCLICK(OrganPanel::OnMouseRightDown)
EVT_MOUSEWHEEL(OrganPanel::OnMouseScroll)
EVT_KEY_DOWN(OrganPanel::OnKeyCommand)
EVT_PAINT(OrganPanel::OnPaint)

END_EVENT_TABLE()

wxPoint g_points[4][17] = {
  { wxPoint( 0, 0), wxPoint(13, 0), wxPoint(13,31), wxPoint( 0,31), wxPoint( 0, 1), wxPoint(12, 1), wxPoint(12,30), wxPoint( 1,30), wxPoint( 1, 1), },
  { wxPoint( 0, 0), wxPoint(10, 0), wxPoint(10,18), wxPoint(13,18), wxPoint(13,31), wxPoint( 0,31), wxPoint( 0, 1), wxPoint( 9, 1), wxPoint( 9,19), wxPoint(12,19), wxPoint(12,30), wxPoint( 1,30), wxPoint( 1, 1), },
  { wxPoint( 3, 0), wxPoint(13, 0), wxPoint(13,31), wxPoint( 0,31), wxPoint( 0,18), wxPoint( 3,18), wxPoint( 3, 1), wxPoint(12, 1), wxPoint(12,30), wxPoint( 1,30), wxPoint( 1,19), wxPoint( 4,19), wxPoint( 4, 1), },
  { wxPoint( 3, 0), wxPoint(10, 0), wxPoint(10,18), wxPoint(13,18), wxPoint(13,31), wxPoint( 0,31), wxPoint( 0,18), wxPoint( 3,18), wxPoint( 3, 1), wxPoint( 9, 1), wxPoint( 9,19), wxPoint(12,19), wxPoint(12,30), wxPoint( 1,30), wxPoint( 1,19), wxPoint( 4,19), wxPoint( 4, 1), },
};

extern GrandOrgueFile* organfile;
extern GOrgueSound* g_sound;
extern const unsigned char* ImageLoader_Wood[];
extern int c_ImageLoader_Wood[];

void TileWood(wxDC& dc, int which, int sx, int sy, int cx, int cy)
{
  int x, y;
  wxMemoryInputStream mem((const char*)ImageLoader_Wood[(which - 1) >> 1], c_ImageLoader_Wood[(which - 1) >> 1]);
  wxImage img(mem, wxBITMAP_TYPE_JPEG);
  if ((which - 1) & 1)
	img = img.Rotate90();
  wxBitmap bmp(img);
  dc.SetClippingRegion(sx, sy, cx, cy);
  for (y = sy & 0xFFFFFF00; y < sy + cy; y += 256)
	for (x = sx & 0xFFFFFF00; x < sx + cx; x += 256)
	  dc.DrawBitmap(bmp, x, y, false);
  dc.DestroyClippingRegion();
}

void OrganPanel::OnUpdate(wxView *WXUNUSED(sender), wxObject *hint)
{
  int i, j, k;
  wxFont font = *wxNORMAL_FONT;

  // if (m_clientBitmap.Ok() && !hint)
  // 	return;

  m_clientOrigin = GetClientAreaOrigin();

  organfile->m_JambLeftRightWidth = organfile->m_DispDrawstopCols * 39;
  if (organfile->m_DispPairDrawstopCols)
	organfile->m_JambLeftRightWidth += ((organfile->m_DispDrawstopCols >> 2) * 18) - 8;
  organfile->m_JambLeftRightHeight = (organfile->m_DispDrawstopRows + 1) * 69;
  organfile->m_JambLeftRightY = (organfile->m_DispScreenSizeVert - organfile->m_JambLeftRightHeight - (organfile->m_DispDrawstopColsOffset ? 35 : 0)) >> 1;
  organfile->m_CenterWidth = organfile->m_JambTopWidth = organfile->m_DispExtraDrawstopCols * 78;
  organfile->m_PistonWidth = organfile->m_DispButtonCols * 44;
  organfile->m_JambTopX = (organfile->m_DispScreenSizeHoriz - organfile->m_JambTopWidth) >> 1;
  organfile->m_PistonX = (organfile->m_DispScreenSizeHoriz - organfile->m_PistonWidth) >> 1;
  organfile->m_CenterWidth=std::max(organfile->m_CenterWidth, organfile->m_PistonWidth);

  organfile->m_CenterY = organfile->m_DispScreenSizeVert - 40;
  for (i = 0; i <= organfile->m_NumberOfManuals; i++)
	{
	  if (!i)
		{
		  organfile->m_manual[0].m_Height = 40;
		  organfile->m_manual[0].m_KeysY = organfile->m_manual[0].m_Y = organfile->m_CenterY;
		  organfile->m_CenterY -= 40;
		  if (organfile->m_DispExtraPedalButtonRow)
			organfile->m_CenterY -= 40;
		  organfile->m_manual[0].m_PistonY = organfile->m_CenterY;
		  organfile->m_CenterY -= 87;	// enclosure area
		  organfile->m_EnclosureWidth = 52 * organfile->m_NumberOfEnclosures;
		  organfile->m_CenterWidth =std::max(organfile->m_CenterWidth, organfile->m_EnclosureWidth);
		  organfile->m_EnclosureY = organfile->m_CenterY + 12;
		}
	  if (!organfile->m_manual[i].Displayed || i < organfile->m_FirstManual)
		continue;

	  if (i)
		{
		  if (!organfile->m_DispButtonsAboveManuals)
			{
			  organfile->m_CenterY -= 40;
			  organfile->m_manual[i].m_PistonY = organfile->m_CenterY;
			}
		  organfile->m_manual[i].m_Height = 32;
		  if (organfile->m_DispTrimBelowManuals && i == 1)
			{
			  organfile->m_manual[i].m_Height += 8;
			  organfile->m_CenterY -= 8;
			}
		  organfile->m_CenterY -= 32;
		  organfile->m_manual[i].m_KeysY = organfile->m_CenterY;
		  if (organfile->m_DispTrimAboveManuals && i == organfile->m_NumberOfManuals)
			{
			  organfile->m_manual[i].m_Height += 8;
			  organfile->m_CenterY -= 8;
			}
		  if (organfile->m_DispButtonsAboveManuals)
			{
			  organfile->m_CenterY -= 40;
			  organfile->m_manual[i].m_PistonY = organfile->m_CenterY;
			}
		  organfile->m_manual[i].m_Y = organfile->m_CenterY;
		}
	  organfile->m_manual[i].m_Width = 1;
	  if (i)
		{
		  for (j = 0; j < organfile->m_manual[i].NumberOfAccessibleKeys; j++)
			{
			  k = (organfile->m_manual[i].FirstAccessibleKeyMIDINoteNumber + j) % 12;
			  if ((k < 5 && !(k & 1)) || (k >= 5 && (k & 1)))
				organfile->m_manual[i].m_Width += 12;
			}
		}
	  else
		{
		  for (j = 0; j < organfile->m_manual[i].NumberOfAccessibleKeys; j++)
			{
			  organfile->m_manual[i].m_Width += 7;
			  k = (organfile->m_manual[i].FirstAccessibleKeyMIDINoteNumber + j) % 12;
			  if (j && (!k || k == 5))
				organfile->m_manual[i].m_Width += 7;
			}
		}
	  organfile->m_manual[i].m_X = (organfile->m_DispScreenSizeHoriz - organfile->m_manual[i].m_Width) >> 1;
	  organfile->m_manual[i].m_Width += 16;
	  organfile->m_CenterWidth=std::max(organfile->m_CenterWidth, organfile->m_manual[i].m_Width);
	}

  /*
	for (; i <= 6; i++)		// evil: for jpOtt
	organfile->m_manual[i].m_PistonY = organfile->m_CenterY - 40;
  */
  organfile->m_HackY = organfile->m_CenterY;

  if (organfile->m_CenterWidth + organfile->m_JambLeftRightWidth * 2 < organfile->m_DispScreenSizeHoriz)
	organfile->m_CenterWidth += (organfile->m_DispScreenSizeHoriz - organfile->m_CenterWidth - organfile->m_JambLeftRightWidth * 2) / 3;

  if (organfile->m_DispExtraDrawstopRowsAboveExtraButtonRows)
	{
	  organfile->m_CenterY -= (organfile->m_PistonTopHeight = organfile->m_DispExtraButtonRows * 40);
	  organfile->m_JambTopPiston = organfile->m_CenterY;
	}
  organfile->m_CenterY -= (organfile->m_JambTopHeight = organfile->m_DispExtraDrawstopRows * 69);
  organfile->m_JambTopDrawstop = organfile->m_CenterY;
  if (!organfile->m_DispExtraDrawstopRowsAboveExtraButtonRows)
	{
	  organfile->m_CenterY -= (organfile->m_PistonTopHeight = organfile->m_DispExtraButtonRows * 40);
	  organfile->m_JambTopPiston = organfile->m_CenterY;
	}
  organfile->m_JambTopY = organfile->m_CenterY;
  if (organfile->m_DispTrimAboveExtraRows)
	organfile->m_CenterY -= 8;
  //TODO: trim should be displayed, blah

  organfile->m_CenterX = (organfile->m_DispScreenSizeHoriz - organfile->m_CenterWidth) >> 1;
  organfile->m_JambLeftX  = (organfile->m_CenterX - organfile->m_JambLeftRightWidth) >> 1;
  organfile->m_JambRightX = organfile->m_JambLeftX + organfile->m_CenterX + organfile->m_CenterWidth;

  m_clientBitmap = wxBitmap(organfile->m_DispScreenSizeHoriz, organfile->m_DispScreenSizeVert);

  wxMemoryDC dc;
  dc.SelectObject(m_clientBitmap);
  TileWood(dc, organfile->m_DispDrawstopBackgroundImageNum, 0, 0, organfile->m_CenterX, organfile->m_DispScreenSizeVert);
  TileWood(dc, organfile->m_DispDrawstopBackgroundImageNum, organfile->m_CenterX + organfile->m_CenterWidth, 0, organfile->m_DispScreenSizeHoriz - (organfile->m_CenterX + organfile->m_CenterWidth), organfile->m_DispScreenSizeVert);
  TileWood(dc, organfile->m_DispConsoleBackgroundImageNum, organfile->m_CenterX, 0, organfile->m_CenterWidth, organfile->m_DispScreenSizeVert);
  if (organfile->m_DispPairDrawstopCols)
	{
	  for (i = 0; i < (organfile->m_DispDrawstopCols >> 2); i++)
		{
		  TileWood(dc, organfile->m_DispDrawstopInsetBackgroundImageNum, i * 174 + organfile->m_JambLeftX,  organfile->m_JambLeftRightY, 166, organfile->m_JambLeftRightHeight);
		  TileWood(dc, organfile->m_DispDrawstopInsetBackgroundImageNum, i * 174 + organfile->m_JambRightX, organfile->m_JambLeftRightY, 166, organfile->m_JambLeftRightHeight);
		}
	  organfile->m_JambLeftX  += 5;
	  organfile->m_JambRightX += 5;
	}
  if (organfile->m_DispTrimAboveExtraRows)
	TileWood(dc, organfile->m_DispKeyVertBackgroundImageNum, organfile->m_CenterX, organfile->m_CenterY, organfile->m_CenterWidth, 8);
  if (organfile->m_JambTopHeight + organfile->m_PistonTopHeight)
	TileWood(dc, organfile->m_DispKeyHorizBackgroundImageNum, organfile->m_CenterX, organfile->m_JambTopY, organfile->m_CenterWidth, organfile->m_JambTopHeight + organfile->m_PistonTopHeight);
  for (i = 0; i <= organfile->m_NumberOfManuals; i++)
	{
	  TileWood(dc, organfile->m_DispKeyVertBackgroundImageNum, organfile->m_CenterX, organfile->m_manual[i].m_Y, organfile->m_CenterWidth, organfile->m_manual[i].m_Height);
	  TileWood(dc, organfile->m_DispKeyHorizBackgroundImageNum, organfile->m_CenterX, organfile->m_manual[i].m_PistonY, organfile->m_CenterWidth, (!i && organfile->m_DispExtraPedalButtonRow) ? 80 : 40);
	  if (i < organfile->m_FirstManual)
		continue;

	  font.SetFaceName(organfile->m_DispControlLabelFont);
	  for (j = 0; j < organfile->m_manual[i].NumberOfStops; j++)
		{
		  if (organfile->m_manual[i].stop[j]->Displayed)
			{
			  font.SetPointSize(organfile->m_manual[i].stop[j]->DispLabelFontSize);
			  dc.SetFont(font);
			  WrapText(dc, organfile->m_manual[i].stop[j]->Name, 51);
			}
		}
	  for (j = 0; j < organfile->m_manual[i].NumberOfCouplers; j++)
		{
		  if (organfile->m_manual[i].coupler[j].Displayed)
			{
			  font.SetPointSize(organfile->m_manual[i].coupler[j].DispLabelFontSize);
			  dc.SetFont(font);
			  WrapText(dc, organfile->m_manual[i].coupler[j].Name, 51);
			}
		}
	  for (j = 0; j < organfile->m_manual[i].NumberOfDivisionals; j++)
		{
		  if (organfile->m_manual[i].divisional[j].Displayed)
			{
			  font.SetPointSize(organfile->m_manual[i].divisional[j].DispLabelFontSize);
			  dc.SetFont(font);
			  WrapText(dc, organfile->m_manual[i].divisional[j].Name, 28);
			}
		}
	  wxRegion region;
	  for (j = 0; j < organfile->m_manual[i].NumberOfAccessibleKeys; j++)
		{
		  k = organfile->m_manual[i].FirstAccessibleKeyMIDINoteNumber + j;
		  if ( (((k % 12) < 5 && !(k & 1)) || ((k % 12) >= 5 && (k & 1))))
			DrawKey(dc, i, j, false, &region);
		}
	  j = 31 + (organfile->m_manual[i].DispKeyColourInverted << 1);
	  if (j == 31 && (organfile->m_manual[i].DispKeyColourWooden || !i))
		j = 35;
	  if (!region.IsEmpty())
		{
		  dc.SetClippingRegion(region);
		  TileWood(dc, j, organfile->m_CenterX, organfile->m_manual[i].m_KeysY, organfile->m_CenterWidth, organfile->m_manual[i].m_Height);
		}
	  region.Clear();
	  for (j = 0; j < organfile->m_manual[i].NumberOfAccessibleKeys; j++)
		{
		  k = organfile->m_manual[i].FirstAccessibleKeyMIDINoteNumber + j;
		  if (!(((k % 12) < 5 && !(k & 1)) || ((k % 12) >= 5 && (k & 1))))
			DrawKey(dc, i, j, false, &region);
		}
	  j = 33 - (organfile->m_manual[i].DispKeyColourInverted << 1);
	  if (j == 31 && (organfile->m_manual[i].DispKeyColourWooden || !i))
		j = (organfile->m_DispKeyVertBackgroundImageNum % 10) == 1 && !i ? 13 : 35;
	  if (!region.IsEmpty())
		{
		  dc.SetClippingRegion(region);
		  TileWood(dc, j, organfile->m_CenterX, organfile->m_manual[i].m_KeysY, organfile->m_CenterWidth, organfile->m_manual[i].m_Height);
		}
	  for (k = 0; k < organfile->m_manual[i].NumberOfAccessibleKeys; k++)
		DrawKey(dc, i, k);
	}
  for (j = 0; j < organfile->m_NumberOfDivisionalCouplers; j++)
	{
	  if (organfile->m_divisionalcoupler[j].Displayed)
		{
		  font.SetPointSize(organfile->m_divisionalcoupler[j].DispLabelFontSize);
		  dc.SetFont(font);
		  WrapText(dc, organfile->m_divisionalcoupler[j].Name, 51);
		}
	}
  for (j = 0; j < organfile->m_NumberOfTremulants; j++)
	{
	  if (organfile->m_tremulant[j].Displayed)
		{
		  font.SetPointSize(organfile->m_tremulant[j].DispLabelFontSize);
		  dc.SetFont(font);
		  WrapText(dc, organfile->m_tremulant[j].Name, 51);
		}
	}
  for (j = 0; j < organfile->m_NumberOfGenerals; j++)
	{
	  if (organfile->m_general[j].Displayed)
		{
		  font.SetPointSize(organfile->m_general[j].DispLabelFontSize);
		  dc.SetFont(font);
		  WrapText(dc, organfile->m_general[j].Name, 28);
		}
	}
  for (j = 0; j < organfile->m_NumberOfReversiblePistons; j++)
	{
	  if (organfile->m_piston[j].Displayed)
		{
		  font.SetPointSize(organfile->m_piston[j].DispLabelFontSize);
		  dc.SetFont(font);
		  WrapText(dc, organfile->m_piston[j].Name, 28);
		}
	}

  j = (organfile->m_DispScreenSizeHoriz - organfile->m_EnclosureWidth + 6) >> 1;
  dc.SetPen(*wxTRANSPARENT_PEN);
  dc.SetBrush(*wxBLACK_BRUSH);
  for (i = 0; i < organfile->m_NumberOfEnclosures; i++)
	{
	  organfile->m_enclosure[i].m_X = j;
	  wxRect rect(organfile->m_enclosure[i].m_X, organfile->m_EnclosureY, 46, 61);
	  dc.DrawRectangle(rect.x, rect.y, rect.width, rect.height);
	  font.SetPointSize(7);
	  dc.SetFont(font);
	  dc.SetTextForeground(*wxWHITE);
	  dc.DrawLabel(organfile->m_enclosure[i].Name, rect, wxALIGN_CENTER_HORIZONTAL);
	  j += 52;
	}

  DrawClickables(&dc);

  for (j = 0; j < organfile->m_NumberOfLabels; j++)
	{
	  if (!organfile->m_label[j].FreeXPlacement)
		{
		  i = organfile->m_DispDrawstopCols >> 1;
		  if (organfile->m_label[j].DispDrawstopCol <= i)
			organfile->m_label[j].DispXpos = organfile->m_JambLeftX  + (organfile->m_label[j].DispDrawstopCol - 1) * 78 + 1;
		  else
			organfile->m_label[j].DispXpos = organfile->m_JambRightX + (organfile->m_label[j].DispDrawstopCol - 1 - i) * 78 + 1;
		  if (organfile->m_label[j].DispSpanDrawstopColToRight)
			organfile->m_label[j].DispXpos += 39;
		}
	  if (!organfile->m_label[j].FreeYPlacement)
		{
		  organfile->m_label[j].DispYpos = organfile->m_JambLeftRightY + 1;
		  if (!organfile->m_label[j].DispAtTopOfDrawstopCol)
			organfile->m_label[j].DispYpos += organfile->m_JambLeftRightHeight - 32;
		}
	  wxRect rect(organfile->m_label[j].DispXpos - 1, organfile->m_label[j].DispYpos - 1, 78, 22);
	  dc.DrawBitmap(organfile->m_images[8], rect.x, rect.y, false);
#ifdef linux
	  font.SetFamily(wxFONTFAMILY_SWISS);
#endif
	  font.SetFaceName(organfile->m_DispGroupLabelFont);
	  font.SetPointSize(organfile->m_label[j].DispLabelFontSize);
	  dc.SetTextForeground(organfile->m_label[j].DispLabelColour);
	  dc.SetFont(font);
	  dc.DrawLabel(organfile->m_label[j].Name, rect, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
	}
  dc.SelectObject(wxNullBitmap);

  GetParent()->SetClientSize(organfile->m_DispScreenSizeHoriz, organfile->m_DispScreenSizeVert);
  SetSize(organfile->m_DispScreenSizeHoriz, organfile->m_DispScreenSizeVert);
  GetParent()->Center(wxBOTH);
  GetParent()->SetBackgroundStyle(wxBG_STYLE_CUSTOM);
  GetParent()->Refresh();
}

void OrganPanel::DrawKey(wxDC& dc, int man, int k, bool usepen, wxRegion* region)
{
  GOrgueManual* manual = &organfile->m_manual[man];

  if (!manual->Displayed)
	return;

  int x, cx, cy, j, z;
  static int addends[12] = {0, 9, 12, 21, 24, 36, 45, 48, 57, 60, 69, 72};

  if (k < 0 || k > manual->NumberOfAccessibleKeys)
	return;

  k += manual->FirstAccessibleKeyMIDINoteNumber;
  z  = (((k % 12) < 5 && !(k & 1)) || ((k % 12) >= 5 && (k & 1))) ? 0 : 1;
  cx = 7;
  cy = 20;
  if (man)
	{
	  x  = manual->m_X + (k / 12) * 84;
	  x += addends[k % 12];
	  j  = manual->FirstAccessibleKeyMIDINoteNumber;
	  x -= (j / 12) * 84;
	  x -= addends[j % 12];
	  if (!z)
		{
		  cx = 13;
		  cy = 32;
		}
	}
  else
	{
	  cx = 8;
	  x  = manual->m_X + (k / 12) * 98;
	  x += (k % 12) * 7;
	  if ((k % 12) >= 5)
		x += 7;
	  j  = manual->FirstAccessibleKeyMIDINoteNumber;
	  x -= (j / 12) * 98;
	  x -= (j % 12) * 7;
	  if ((j % 12) >= 5)
		x -= 7;
	  if (!z)
		cy = 40;
	}

  wxRegion reg;
  if (!man || z)
	{
	  reg.Union(x, manual->m_KeysY, cx + 1, cy);
	  z = -4;
	}
  else
	{
	  reg.Union(x + 3, manual->m_KeysY, 8, cy);
	  reg.Union(x, manual->m_KeysY + 18, 14, 14);
	  z = 0;
	  j = k % 12;
	  if (k > manual->FirstAccessibleKeyMIDINoteNumber && j && j != 5)
		z |= 2;
	  if (k < manual->FirstAccessibleKeyMIDINoteNumber + manual->NumberOfAccessibleKeys - 1 && j != 4 && j != 11)
		z |= 1;

	  if (!(z & 2))
		reg.Union(x, manual->m_KeysY, 3, 18);
	  if (!(z & 1))
		reg.Union(x + 11, manual->m_KeysY, 3, 18);
	}

  if (region)
	region->Union(reg);
  if (usepen)
	{
	  k -= manual->FirstAccessibleKeyMIDINoteNumber;
	  const wxPen* pen = manual->m_MIDI[k] ? wxRED_PEN : wxGREY_PEN;
	  dc.SetPen(*pen);
	  wxRegion exclude;
	  if (k > 0 && manual->m_MIDI[k - 1])
		DrawKey(dc, man, k - 1, 0, &exclude);
	  if ((z & 2) && k > 1 && manual->m_MIDI[k - 2])
		DrawKey(dc, man, k - 2, 0, &exclude);
	  if (k < manual->NumberOfAccessibleKeys - 1 && manual->m_MIDI[k + 1])
		DrawKey(dc, man, k + 1, 0, &exclude);
	  if ((z & 1) && k < manual->NumberOfAccessibleKeys - 2 && manual->m_MIDI[k + 2])
		DrawKey(dc, man, k + 2, 0, &exclude);
	  if (!exclude.IsEmpty())
		{
		  reg.Subtract(exclude);
		  reg.Offset(dc.LogicalToDeviceX(0), dc.LogicalToDeviceY(0));
		  dc.SetClippingRegion(reg);
		}
	  if (z < 0)
		{
		  dc.SetBrush(*wxTRANSPARENT_BRUSH);
		  dc.DrawRectangle(x, manual->m_KeysY, cx + 1, cy);
		  dc.DrawRectangle(x + 1, manual->m_KeysY + 1, cx - 1, cy - 2);
		}
	  else
		{
		  dc.DrawPolygon(9 + (((z + 1) >> 1) << 2), g_points[z], x, manual->m_KeysY);
		}
	  if (!exclude.IsEmpty())
		dc.DestroyClippingRegion();
	}
}

void OrganPanel::OnErase(wxEraseEvent& event)
{
  if (!m_clientBitmap.Ok() || !organfile || !organfile->m_JambLeftRightWidth)
	{
	  event.Skip();
	  return;
	}
  wxDC* dc = event.GetDC();
  OnDraw(dc);
}
void OrganPanel::OnPaint(wxPaintEvent& event)
{
  if (!m_clientBitmap.Ok() || !organfile || !organfile->m_JambLeftRightWidth)
	{
	  event.Skip();
	  return;
	}
  wxPaintDC dc(this);
  OnDraw((wxDC*)&dc);
}

void OrganPanel::OnDrawstop(wxCommandEvent& event)
{
  if (!m_clientBitmap.Ok() || !organfile || !organfile->m_JambLeftRightWidth)
	return;

  wxMemoryDC mdc;
  mdc.SelectObject(m_clientBitmap);
  wxClientDC dc(this);
  dc.SetDeviceOrigin(m_clientOrigin.x, m_clientOrigin.y);

  static_cast<GOrgueDrawable*>(event.GetClientData())->Draw(0, 0, &mdc, &dc);
}

void OrganPanel::OnNoteOnOff(wxCommandEvent& event)
{
  if (!m_clientBitmap.Ok() || !organfile || !organfile->m_JambLeftRightWidth)
	return;

  wxMemoryDC mdc;
  mdc.SelectObject(m_clientBitmap);
  wxClientDC dc(this);
  dc.SetDeviceOrigin(m_clientOrigin.x, m_clientOrigin.y);
  DrawKey(mdc, event.GetInt(), event.GetExtraLong(), true);
  DrawKey( dc, event.GetInt(), event.GetExtraLong(), true);
}

void OrganPanel::OnMouseLeftDown(wxMouseEvent& event)
{
  wxClientDC dc(this);
  DrawClickables(NULL, event.GetX(), event.GetY());
  event.Skip();
}

void OrganPanel::OnMouseRightDown(wxMouseEvent& event)
{
  wxClientDC dc(this);
  DrawClickables(NULL, event.GetX(), event.GetY(), true);
  event.Skip();
}

void OrganPanel::OnMouseScroll(wxMouseEvent& event)
{
  wxClientDC dc(this);
  DrawClickables(NULL, event.GetX() + m_clientOrigin.x, event.GetY() + m_clientOrigin.y, false, event.GetWheelRotation());
  event.Skip();
}


void OrganPanel::HelpDrawStop(GOrgueDrawstop* stop, wxDC* dc, int xx, int yy, bool right)
{
  if (stop->Draw(xx, yy, dc))
    {
	  if (right)
		stop->MIDI();
	  else
		{
		stop->Push();
		wxMemoryDC mdc;
		mdc.SelectObject(m_clientBitmap);
		wxClientDC dc(this);
		dc.SetDeviceOrigin(m_clientOrigin.x, m_clientOrigin.y);

		stop->Draw(0, 0, &mdc, &dc);

		}
    }
}

void OrganPanel::HelpDrawButton(GOrguePushbutton* button, wxDC* dc, int xx, int yy, bool right)
{
  if (button->Draw(xx, yy, dc))
    {
	  if (right)
		button->MIDI();
	  else
		{
		button->Push();
		wxMemoryDC mdc;
		mdc.SelectObject(m_clientBitmap);
		wxClientDC dc(this);
		dc.SetDeviceOrigin(m_clientOrigin.x, m_clientOrigin.y);

		button->Draw(0, 0, &mdc, &dc);
		}
    }
}

void OrganPanel::DrawClickables(wxDC* dc, int xx, int yy, bool right, int scroll)
{
  int i, j;
  if (!m_clientBitmap.Ok())
	return;

  if (!scroll)
    {
	  for (i = organfile->m_FirstManual; i <= organfile->m_NumberOfManuals; i++)
        {
		  for (j = 0; j < organfile->m_manual[i].NumberOfStops; j++)
			HelpDrawStop(organfile->m_manual[i].stop[j], dc, xx, yy, right);
		  for (j = 0; j < organfile->m_manual[i].NumberOfCouplers; j++)
			HelpDrawStop(&organfile->m_manual[i].coupler[j], dc, xx, yy, right);
		  for (j = 0; j < organfile->m_manual[i].NumberOfDivisionals; j++)
			HelpDrawButton(&organfile->m_manual[i].divisional[j], dc, xx, yy, right);
		  if (dc || !organfile->m_manual[i].Displayed)
			continue;
		  wxRect rect(organfile->m_manual[i].m_X, organfile->m_manual[i].m_Y, organfile->m_manual[i].m_Width, organfile->m_manual[i].m_Height);
		  if (rect.Contains(xx, yy))
            {
			  if (right)
				organfile->m_manual[i].MIDI();
			  else
				;
            }
        }
	  for (j = 0; j < organfile->m_NumberOfTremulants; j++)
		HelpDrawStop(&organfile->m_tremulant[j], dc, xx, yy, right);
	  for (j = 0; j < organfile->m_NumberOfDivisionalCouplers; j++)
		HelpDrawStop(&organfile->m_divisionalcoupler[j], dc, xx, yy, right);
	  for (j = 0; j < organfile->m_NumberOfGenerals; j++)
		HelpDrawButton(&organfile->m_general[j], dc, xx, yy, right);
	  for (j = 0; j < organfile->m_NumberOfReversiblePistons; j++)
		HelpDrawButton(&organfile->m_piston[j], dc, xx, yy, right);
    }

  for (i = 0; i < organfile->m_NumberOfEnclosures; i++)
	{
	  if (organfile->m_enclosure[i].Draw(xx, yy, dc))
        {
		  if (right)
			organfile->m_enclosure[i].MIDI();
		  else if (scroll)
			organfile->m_enclosure[i].Set(organfile->m_enclosure[i].MIDIValue + (scroll > 0 ? 16 : -16));
        }
	}
}

void OrganPanel::WrapText(wxDC& dc, wxString& string, int width)
{
  wxString str = string;

  char *ptr = (char*)str.c_str();
  char *p = ptr, *lastspace = 0;
  wxCoord cx, cy;

  while (p)
	{
	  p = strchr(p, ' ');
	  if (p)
		{
		  *p = 0;
		  dc.GetTextExtent(ptr, &cx, &cy);
		  *p = ' ';
		}
	  else
		dc.GetTextExtent(ptr, &cx, &cy);
	  if (cx > width)
		{
		  if (lastspace)
			{
			  *lastspace = '\n';
			  if (p)
				ptr = p = lastspace + 1;
			}
		  else
			{
			  if (p)
				*p++ = '\n';
			  ptr = p;
			}
		  lastspace = 0;
		}
	  else if (p)
		lastspace = p++;
	}

  lastspace = 0;
  p = ptr = (char*)str.c_str();
  while (*p)
	{
	  if (*p == ' ')
		{
		  if (!lastspace)
			lastspace = ptr;
		}
	  else
		{
		  if (*p == '\n' && lastspace)
			ptr = lastspace;
		  lastspace = 0;
		}
	  *ptr++ = *p++;
	}
  *ptr = 0;

  string = str.c_str();
}


void OrganPanel::OnKeyCommand(wxKeyEvent& event)
{
	if (g_sound && g_sound->b_memset ^ event.ShiftDown())
	{
	  ::wxGetApp().frame->ProcessCommand(ID_AUDIO_MEMSET);
	  UpdateWindowUI();
	}

	int i, j;
	int k = event.GetKeyCode();
    if ( !event.AltDown())
    {

        GOrgueMeter* meter = ::wxGetApp().frame->m_meters[2];
        OrganDocument* doc = (OrganDocument*)::wxGetApp().m_docManager->GetCurrentDocument();
        switch(k)
        {
        case WXK_ESCAPE:
		  {
			::wxGetApp().frame->ProcessCommand(ID_AUDIO_PANIC);
            break;
		  }
        case WXK_LEFT:
		  {
            meter->SetValue(meter->GetValue() - 1);
            break;
		  }
        case WXK_DOWN:
		  {
            if (organfile)
                organfile->m_framegeneral[meter->GetValue() - 1].Push();
            break;
		  }
        case WXK_RIGHT:
		  {
            meter->SetValue(meter->GetValue() + 1);
            break;
		  }
        default:
		  {
            if (organfile && doc && doc->b_loaded && (k = WXKtoVK(k)))
            {
                for (i = organfile->m_FirstManual; i <= organfile->m_NumberOfManuals; i++)
                {
                    for (j = 0; j < organfile->m_manual[i].NumberOfStops; j++)
					  {
                        if (k == organfile->m_manual[i].stop[j]->ShortcutKey)
                            organfile->m_manual[i].stop[j]->Push();
					  }
                    for (j = 0; j < organfile->m_manual[i].NumberOfCouplers; j++)
                        if (k == organfile->m_manual[i].coupler[j].ShortcutKey)
                            organfile->m_manual[i].coupler[j].Push();
                    for (j = 0; j < organfile->m_manual[i].NumberOfDivisionals; j++)
                        if (k == organfile->m_manual[i].divisional[j].ShortcutKey)
                            organfile->m_manual[i].divisional[j].Push();
                }
                for (j = 0; j < organfile->m_NumberOfTremulants; j++)
                    if (k == organfile->m_tremulant[j].ShortcutKey)
                        organfile->m_tremulant[j].Push();
                for (j = 0; j < organfile->m_NumberOfDivisionalCouplers; j++)
                    if (k == organfile->m_divisionalcoupler[j].ShortcutKey)
                        organfile->m_divisionalcoupler[j].Push();
                for (j = 0; j < organfile->m_NumberOfGenerals; j++)
                    if (k == organfile->m_general[j].ShortcutKey)
                        organfile->m_general[j].Push();
                for (j = 0; j < organfile->m_NumberOfReversiblePistons; j++)
                    if (k == organfile->m_piston[j].ShortcutKey)
                        organfile->m_piston[j].Push();
            }
            event.Skip();
		  }
        }
    }
    else
		event.Skip();
	event.Skip();
}
