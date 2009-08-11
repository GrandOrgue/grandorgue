/*
 * GrandOrgue - Copyright (C) 2009 JLDER - free pipe organ simulator based on MyOrgan Copyright (C) 2006 Kloria Publishing LLC
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

#include "MyOrgan.h"

IMPLEMENT_DYNAMIC_CLASS(OrganView, wxView)

extern void MIDIStopChange(MyStop& stop, bool on, int manual);
extern void MIDICouplerChange(MyCoupler& stop, bool on);
extern void MIDIDivisionalCouplerChange(MyDivisionalCoupler& stop, bool on);
extern void MIDIDivisionalPush(int manual, int which, int depth = 0);
extern void MIDIGeneralPush(MyGeneral& general);

#define DOMAX(a,b) a=((a)>(b)?(a):(b))

extern MyOrganFile* organfile;
extern const unsigned char* Images_Wood[];
extern int c_Images_Wood[];

wxPoint g_points[4][17] = {
	{ wxPoint( 0, 0), wxPoint(13, 0), wxPoint(13,31), wxPoint( 0,31), wxPoint( 0, 1), wxPoint(12, 1), wxPoint(12,30), wxPoint( 1,30), wxPoint( 1, 1), },
	{ wxPoint( 0, 0), wxPoint(10, 0), wxPoint(10,18), wxPoint(13,18), wxPoint(13,31), wxPoint( 0,31), wxPoint( 0, 1), wxPoint( 9, 1), wxPoint( 9,19), wxPoint(12,19), wxPoint(12,30), wxPoint( 1,30), wxPoint( 1, 1), },
	{ wxPoint( 3, 0), wxPoint(13, 0), wxPoint(13,31), wxPoint( 0,31), wxPoint( 0,18), wxPoint( 3,18), wxPoint( 3, 1), wxPoint(12, 1), wxPoint(12,30), wxPoint( 1,30), wxPoint( 1,19), wxPoint( 4,19), wxPoint( 4, 1), },
	{ wxPoint( 3, 0), wxPoint(10, 0), wxPoint(10,18), wxPoint(13,18), wxPoint(13,31), wxPoint( 0,31), wxPoint( 0,18), wxPoint( 3,18), wxPoint( 3, 1), wxPoint( 9, 1), wxPoint( 9,19), wxPoint(12,19), wxPoint(12,30), wxPoint( 1,30), wxPoint( 1,19), wxPoint( 4,19), wxPoint( 4, 1), },
};

BEGIN_EVENT_TABLE(OrganView, wxView)
	EVT_ERASE_BACKGROUND(OrganView::OnErase)
	EVT_COMMAND(0, wxEVT_DRAWSTOP, OrganView::OnDrawstop)
	EVT_COMMAND(0, wxEVT_PUSHBUTTON, OrganView::OnDrawstop)
	EVT_COMMAND(0, wxEVT_ENCLOSURE, OrganView::OnDrawstop)
	EVT_COMMAND(0, wxEVT_NOTEONOFF, OrganView::OnNoteOnOff)
	EVT_LEFT_DOWN(OrganView::OnMouseLeftDown)
	EVT_LEFT_DCLICK(OrganView::OnMouseLeftDown)
	EVT_RIGHT_DOWN(OrganView::OnMouseRightDown)
	EVT_RIGHT_DCLICK(OrganView::OnMouseRightDown)
	EVT_MOUSEWHEEL(OrganView::OnMouseScroll)
END_EVENT_TABLE()

void TileWood(wxDC& dc, int which, int sx, int sy, int cx, int cy)
{
	int x, y;
	wxMemoryInputStream mem((const char*)Images_Wood[(which - 1) >> 1], c_Images_Wood[(which - 1) >> 1]);
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

bool OrganView::OnCreate(wxDocument *doc, long flags)
{
	SetFrame(::wxGetApp().frame);
	Activate(true);
	GetFrame()->SetFocus();

	return true;
}

void OrganView::OnUpdate(wxView *WXUNUSED(sender), wxObject *hint)
{
	int i, j, k;
	wxFont font = *wxNORMAL_FONT;

	if (m_clientBitmap.Ok() && !hint)
		return;

	m_clientOrigin = GetFrame()->GetClientAreaOrigin();

	organfile->m_JambLeftRightWidth = organfile->DispDrawstopCols * 39;
	if (organfile->DispPairDrawstopCols)
		organfile->m_JambLeftRightWidth += ((organfile->DispDrawstopCols >> 2) * 18) - 8;
	organfile->m_JambLeftRightHeight = (organfile->DispDrawstopRows + 1) * 69;
	organfile->m_JambLeftRightY = (organfile->DispScreenSizeVert - organfile->m_JambLeftRightHeight - (organfile->DispDrawstopColsOffset ? 35 : 0)) >> 1;
	organfile->m_CenterWidth = organfile->m_JambTopWidth = organfile->DispExtraDrawstopCols * 78;
	organfile->m_PistonWidth = organfile->DispButtonCols * 44;
	organfile->m_JambTopX = (organfile->DispScreenSizeHoriz - organfile->m_JambTopWidth) >> 1;
	organfile->m_PistonX = (organfile->DispScreenSizeHoriz - organfile->m_PistonWidth) >> 1;
	DOMAX(organfile->m_CenterWidth, organfile->m_PistonWidth);

	organfile->m_CenterY = organfile->DispScreenSizeVert - 40;
	for (i = 0; i <= organfile->NumberOfManuals; i++)
	{
		if (!i)
		{
			organfile->manual[0].m_Height = 40;
			organfile->manual[0].m_KeysY = organfile->manual[0].m_Y = organfile->m_CenterY;
			organfile->m_CenterY -= 40;
			if (organfile->DispExtraPedalButtonRow)
				organfile->m_CenterY -= 40;
			organfile->manual[0].m_PistonY = organfile->m_CenterY;
			organfile->m_CenterY -= 87;	// enclosure area
			organfile->m_EnclosureWidth = 52 * organfile->NumberOfEnclosures;
			DOMAX(organfile->m_CenterWidth, organfile->m_EnclosureWidth);
			organfile->m_EnclosureY = organfile->m_CenterY + 12;
		}
		if (!organfile->manual[i].Displayed || i < organfile->FirstManual)
			continue;

		if (i)
		{
			if (!organfile->DispButtonsAboveManuals)
			{
				organfile->m_CenterY -= 40;
				organfile->manual[i].m_PistonY = organfile->m_CenterY;
			}
			organfile->manual[i].m_Height = 32;
			if (organfile->DispTrimBelowManuals && i == 1)
			{
				organfile->manual[i].m_Height += 8;
				organfile->m_CenterY -= 8;
			}
			organfile->m_CenterY -= 32;
			organfile->manual[i].m_KeysY = organfile->m_CenterY;
			if (organfile->DispTrimAboveManuals && i == organfile->NumberOfManuals)
			{
				organfile->manual[i].m_Height += 8;
				organfile->m_CenterY -= 8;
			}
			if (organfile->DispButtonsAboveManuals)
			{
				organfile->m_CenterY -= 40;
				organfile->manual[i].m_PistonY = organfile->m_CenterY;
			}
			organfile->manual[i].m_Y = organfile->m_CenterY;
		}
		organfile->manual[i].m_Width = 1;
		if (i)
		{
			for (j = 0; j < organfile->manual[i].NumberOfAccessibleKeys; j++)
			{
				k = (organfile->manual[i].FirstAccessibleKeyMIDINoteNumber + j) % 12;
				if ((k < 5 && !(k & 1)) || (k >= 5 && (k & 1)))
					organfile->manual[i].m_Width += 12;
			}
		}
		else
		{
			for (j = 0; j < organfile->manual[i].NumberOfAccessibleKeys; j++)
			{
				organfile->manual[i].m_Width += 7;
				k = (organfile->manual[i].FirstAccessibleKeyMIDINoteNumber + j) % 12;
				if (j && (!k || k == 5))
					organfile->manual[i].m_Width += 7;
			}
		}
		organfile->manual[i].m_X = (organfile->DispScreenSizeHoriz - organfile->manual[i].m_Width) >> 1;
		organfile->manual[i].m_Width += 16;
		DOMAX(organfile->m_CenterWidth, organfile->manual[i].m_Width);
	}

	/*
	for (; i <= 6; i++)		// evil: for jpOtt
		organfile->manual[i].m_PistonY = organfile->m_CenterY - 40;
    */
    organfile->m_HackY = organfile->m_CenterY;

	if (organfile->m_CenterWidth + organfile->m_JambLeftRightWidth * 2 < organfile->DispScreenSizeHoriz)
        organfile->m_CenterWidth += (organfile->DispScreenSizeHoriz - organfile->m_CenterWidth - organfile->m_JambLeftRightWidth * 2) / 3;

	if (organfile->DispExtraDrawstopRowsAboveExtraButtonRows)
	{
		organfile->m_CenterY -= (organfile->m_PistonTopHeight = organfile->DispExtraButtonRows * 40);
		organfile->m_JambTopPiston = organfile->m_CenterY;
	}
	organfile->m_CenterY -= (organfile->m_JambTopHeight = organfile->DispExtraDrawstopRows * 69);
	organfile->m_JambTopDrawstop = organfile->m_CenterY;
	if (!organfile->DispExtraDrawstopRowsAboveExtraButtonRows)
	{
		organfile->m_CenterY -= (organfile->m_PistonTopHeight = organfile->DispExtraButtonRows * 40);
		organfile->m_JambTopPiston = organfile->m_CenterY;
	}
	organfile->m_JambTopY = organfile->m_CenterY;
	if (organfile->DispTrimAboveExtraRows)
		organfile->m_CenterY -= 8;
	//TODO: trim should be displayed, blah

	organfile->m_CenterX = (organfile->DispScreenSizeHoriz - organfile->m_CenterWidth) >> 1;
	organfile->m_JambLeftX  = (organfile->m_CenterX - organfile->m_JambLeftRightWidth) >> 1;
	organfile->m_JambRightX = organfile->m_JambLeftX + organfile->m_CenterX + organfile->m_CenterWidth;

	m_clientBitmap = wxBitmap(organfile->DispScreenSizeHoriz, organfile->DispScreenSizeVert);

	wxMemoryDC dc;
	dc.SelectObject(m_clientBitmap);
	TileWood(dc, organfile->DispDrawstopBackgroundImageNum, 0, 0, organfile->m_CenterX, organfile->DispScreenSizeVert);
	TileWood(dc, organfile->DispDrawstopBackgroundImageNum, organfile->m_CenterX + organfile->m_CenterWidth, 0, organfile->DispScreenSizeHoriz - (organfile->m_CenterX + organfile->m_CenterWidth), organfile->DispScreenSizeVert);
	TileWood(dc, organfile->DispConsoleBackgroundImageNum, organfile->m_CenterX, 0, organfile->m_CenterWidth, organfile->DispScreenSizeVert);
	if (organfile->DispPairDrawstopCols)
	{
		for (i = 0; i < (organfile->DispDrawstopCols >> 2); i++)
		{
			TileWood(dc, organfile->DispDrawstopInsetBackgroundImageNum, i * 174 + organfile->m_JambLeftX,  organfile->m_JambLeftRightY, 166, organfile->m_JambLeftRightHeight);
			TileWood(dc, organfile->DispDrawstopInsetBackgroundImageNum, i * 174 + organfile->m_JambRightX, organfile->m_JambLeftRightY, 166, organfile->m_JambLeftRightHeight);
		}
		organfile->m_JambLeftX  += 5;
		organfile->m_JambRightX += 5;
	}
	if (organfile->DispTrimAboveExtraRows)
        TileWood(dc, organfile->DispKeyVertBackgroundImageNum, organfile->m_CenterX, organfile->m_CenterY, organfile->m_CenterWidth, 8);
	if (organfile->m_JambTopHeight + organfile->m_PistonTopHeight)
        TileWood(dc, organfile->DispKeyHorizBackgroundImageNum, organfile->m_CenterX, organfile->m_JambTopY, organfile->m_CenterWidth, organfile->m_JambTopHeight + organfile->m_PistonTopHeight);
	for (i = 0; i <= organfile->NumberOfManuals; i++)
	{
		TileWood(dc, organfile->DispKeyVertBackgroundImageNum, organfile->m_CenterX, organfile->manual[i].m_Y, organfile->m_CenterWidth, organfile->manual[i].m_Height);
		TileWood(dc, organfile->DispKeyHorizBackgroundImageNum, organfile->m_CenterX, organfile->manual[i].m_PistonY, organfile->m_CenterWidth, (!i && organfile->DispExtraPedalButtonRow) ? 80 : 40);
		if (i < organfile->FirstManual)
			continue;

		font.SetFaceName(organfile->DispControlLabelFont);
		for (j = 0; j < organfile->manual[i].NumberOfStops; j++)
		{
			if (organfile->manual[i].stop[j].Displayed)
			{
				font.SetPointSize(organfile->manual[i].stop[j].DispLabelFontSize);
				dc.SetFont(font);
				WrapText(dc, organfile->manual[i].stop[j].Name, 51);
			}
		}
		for (j = 0; j < organfile->manual[i].NumberOfCouplers; j++)
		{
			if (organfile->manual[i].coupler[j].Displayed)
			{
				font.SetPointSize(organfile->manual[i].coupler[j].DispLabelFontSize);
				dc.SetFont(font);
				WrapText(dc, organfile->manual[i].coupler[j].Name, 51);
			}
		}
		for (j = 0; j < organfile->manual[i].NumberOfDivisionals; j++)
		{
			if (organfile->manual[i].divisional[j].Displayed)
			{
				font.SetPointSize(organfile->manual[i].divisional[j].DispLabelFontSize);
				dc.SetFont(font);
				WrapText(dc, organfile->manual[i].divisional[j].Name, 28);
			}
		}
		wxRegion region;
		for (j = 0; j < organfile->manual[i].NumberOfAccessibleKeys; j++)
		{
			k = organfile->manual[i].FirstAccessibleKeyMIDINoteNumber + j;
			if ( (((k % 12) < 5 && !(k & 1)) || ((k % 12) >= 5 && (k & 1))))
				DrawKey(dc, i, j, false, &region);
		}
		j = 31 + (organfile->manual[i].DispKeyColourInverted << 1);
		if (j == 31 && (organfile->manual[i].DispKeyColourWooden || !i))
			j = 35;
		if (!region.IsEmpty())
		{
			dc.SetClippingRegion(region);
			TileWood(dc, j, organfile->m_CenterX, organfile->manual[i].m_KeysY, organfile->m_CenterWidth, organfile->manual[i].m_Height);
		}
		region.Clear();
		for (j = 0; j < organfile->manual[i].NumberOfAccessibleKeys; j++)
		{
			k = organfile->manual[i].FirstAccessibleKeyMIDINoteNumber + j;
			if (!(((k % 12) < 5 && !(k & 1)) || ((k % 12) >= 5 && (k & 1))))
				DrawKey(dc, i, j, false, &region);
		}
		j = 33 - (organfile->manual[i].DispKeyColourInverted << 1);
		if (j == 31 && (organfile->manual[i].DispKeyColourWooden || !i))
			j = (organfile->DispKeyVertBackgroundImageNum % 10) == 1 && !i ? 13 : 35;
		if (!region.IsEmpty())
		{
			dc.SetClippingRegion(region);
			TileWood(dc, j, organfile->m_CenterX, organfile->manual[i].m_KeysY, organfile->m_CenterWidth, organfile->manual[i].m_Height);
		}
		for (k = 0; k < organfile->manual[i].NumberOfAccessibleKeys; k++)
			DrawKey(dc, i, k);
	}
	for (j = 0; j < organfile->NumberOfDivisionalCouplers; j++)
	{
		if (organfile->divisionalcoupler[j].Displayed)
		{
			font.SetPointSize(organfile->divisionalcoupler[j].DispLabelFontSize);
			dc.SetFont(font);
			WrapText(dc, organfile->divisionalcoupler[j].Name, 51);
		}
	}
	for (j = 0; j < organfile->NumberOfTremulants; j++)
	{
		if (organfile->tremulant[j].Displayed)
		{
			font.SetPointSize(organfile->tremulant[j].DispLabelFontSize);
			dc.SetFont(font);
			WrapText(dc, organfile->tremulant[j].Name, 51);
		}
	}
	for (j = 0; j < organfile->NumberOfGenerals; j++)
	{
		if (organfile->general[j].Displayed)
		{
			font.SetPointSize(organfile->general[j].DispLabelFontSize);
			dc.SetFont(font);
			WrapText(dc, organfile->general[j].Name, 28);
		}
	}
	for (j = 0; j < organfile->NumberOfReversiblePistons; j++)
	{
		if (organfile->piston[j].Displayed)
		{
			font.SetPointSize(organfile->piston[j].DispLabelFontSize);
			dc.SetFont(font);
			WrapText(dc, organfile->piston[j].Name, 28);
		}
	}

	j = (organfile->DispScreenSizeHoriz - organfile->m_EnclosureWidth + 6) >> 1;
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(*wxBLACK_BRUSH);
	for (i = 0; i < organfile->NumberOfEnclosures; i++)
	{
		organfile->enclosure[i].m_X = j;
		wxRect rect(organfile->enclosure[i].m_X, organfile->m_EnclosureY, 46, 61);
		dc.DrawRectangle(rect.x, rect.y, rect.width, rect.height);
		font.SetPointSize(7);
		dc.SetFont(font);
		dc.SetTextForeground(*wxWHITE);
		dc.DrawLabel(organfile->enclosure[i].Name, rect, wxALIGN_CENTER_HORIZONTAL);
		j += 52;
	}

	DrawClickables(&dc);

	for (j = 0; j < organfile->NumberOfLabels; j++)
	{
		if (!organfile->label[j].FreeXPlacement)
		{
			i = organfile->DispDrawstopCols >> 1;
			if (organfile->label[j].DispDrawstopCol <= i)
				organfile->label[j].DispXpos = organfile->m_JambLeftX  + (organfile->label[j].DispDrawstopCol - 1) * 78 + 1;
			else
				organfile->label[j].DispXpos = organfile->m_JambRightX + (organfile->label[j].DispDrawstopCol - 1 - i) * 78 + 1;
			if (organfile->label[j].DispSpanDrawstopColToRight)
                organfile->label[j].DispXpos += 39;
		}
		if (!organfile->label[j].FreeYPlacement)
		{
			organfile->label[j].DispYpos = organfile->m_JambLeftRightY + 1;
			if (!organfile->label[j].DispAtTopOfDrawstopCol)
				organfile->label[j].DispYpos += organfile->m_JambLeftRightHeight - 32;
		}
		wxRect rect(organfile->label[j].DispXpos - 1, organfile->label[j].DispYpos - 1, 78, 22);
		dc.DrawBitmap(organfile->m_images[8], rect.x, rect.y, false);
		#ifdef linux
		   font.SetFamily(wxFONTFAMILY_SWISS);
		#endif
		font.SetFaceName(organfile->DispGroupLabelFont);
		font.SetPointSize(organfile->label[j].DispLabelFontSize);
		dc.SetTextForeground(organfile->label[j].DispLabelColour);
		dc.SetFont(font);
		dc.DrawLabel(organfile->label[j].Name, rect, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
	}
	dc.SelectObject(wxNullBitmap);

	GetFrame()->SetClientSize(organfile->DispScreenSizeHoriz, organfile->DispScreenSizeVert);
	GetFrame()->Center(wxBOTH);

	GetFrame()->SetBackgroundStyle(wxBG_STYLE_CUSTOM);
	GetFrame()->Refresh();
}

void OrganView::DrawKey(wxDC& dc, int man, int k, bool usepen, wxRegion* region)
{
	MyManual* manual = &organfile->manual[man];

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

void OrganView::OnErase(wxEraseEvent& event)
{
	if (!m_clientBitmap.Ok() || !organfile || !organfile->m_JambLeftRightWidth)
	{
	    event.Skip();
		return;
	}
    wxDC* dc = event.GetDC();
    OnDraw(dc);
}

void OrganView::OnPaint(wxPaintEvent& event)
{
	if (!m_clientBitmap.Ok() || !organfile || !organfile->m_JambLeftRightWidth)
	{
	    event.Skip();
		return;
	}
	wxPaintDC dc(GetFrame());
	OnDraw((wxDC*)&dc);
}

void OrganView::OnDraw(wxDC* dc)
{
	if (!m_clientBitmap.Ok() || !organfile || !organfile->m_JambLeftRightWidth)
        return;
    dc->DrawBitmap(m_clientBitmap, m_clientOrigin.x, m_clientOrigin.y, false);
}

void OrganView::OnDrawstop(wxCommandEvent& event)
{
	if (!m_clientBitmap.Ok() || !organfile || !organfile->m_JambLeftRightWidth)
		return;

	wxMemoryDC mdc;
	mdc.SelectObject(m_clientBitmap);
	wxClientDC dc(GetFrame());
	dc.SetDeviceOrigin(m_clientOrigin.x, m_clientOrigin.y);

	if (event.GetEventType() == wxEVT_DRAWSTOP)
        ((MyDrawstop*)event.GetClientData())->Draw(0, 0, &mdc, &dc);
	else if (event.GetEventType() == wxEVT_PUSHBUTTON)
        ((MyPushbutton*)event.GetClientData())->Draw(0, 0, &mdc, &dc);
    else
        ((MyEnclosure*)event.GetClientData())->Draw(0, 0, &mdc, &dc);
}

void OrganView::OnNoteOnOff(wxCommandEvent& event)
{
	if (!m_clientBitmap.Ok() || !organfile || !organfile->m_JambLeftRightWidth)
		return;

	wxMemoryDC mdc;
	mdc.SelectObject(m_clientBitmap);
	wxClientDC dc(GetFrame());
	dc.SetDeviceOrigin(m_clientOrigin.x, m_clientOrigin.y);
	DrawKey(mdc, event.GetInt(), event.GetExtraLong(), true);
	DrawKey( dc, event.GetInt(), event.GetExtraLong(), true);
}

void OrganView::OnMouseLeftDown(wxMouseEvent& event)
{
	DrawClickables(NULL, event.GetX(), event.GetY());
	event.Skip();
}

void OrganView::OnMouseRightDown(wxMouseEvent& event)
{
	DrawClickables(NULL, event.GetX(), event.GetY(), true);
	event.Skip();
}

void OrganView::OnMouseScroll(wxMouseEvent& event)
{
	DrawClickables(NULL, event.GetX() + m_clientOrigin.x, event.GetY() + m_clientOrigin.y, false, event.GetWheelRotation());
	event.Skip();
}

bool OrganView::OnClose(bool deleteWindow)
{
	GetFrame()->SetBackgroundStyle(wxBG_STYLE_SYSTEM);
	GetFrame()->Refresh();
    if (!GetDocument()->Close())
        return false;

	SetFrame(0);
	Activate(false);

	return true;
}

inline void HelpDrawStop(MyDrawstop* stop, wxDC* dc, int xx, int yy, bool right)
{
    if (stop->Draw(xx, yy, dc))
    {
        if (right)
            stop->MIDI();
        else
            stop->Push();
    }
}

inline void HelpDrawButton(MyPushbutton* button, wxDC* dc, int xx, int yy, bool right)
{
    if (button->Draw(xx, yy, dc))
    {
        if (right)
            button->MIDI();
        else
            button->Push();
    }
}

void OrganView::DrawClickables(wxDC* dc, int xx, int yy, bool right, int scroll)
{
	int i, j;
	if (!m_clientBitmap.Ok())
		return;

    if (!scroll)
    {
        for (i = organfile->FirstManual; i <= organfile->NumberOfManuals; i++)
        {
            for (j = 0; j < organfile->manual[i].NumberOfStops; j++)
                HelpDrawStop(&organfile->manual[i].stop[j], dc, xx, yy, right);
            for (j = 0; j < organfile->manual[i].NumberOfCouplers; j++)
                HelpDrawStop(&organfile->manual[i].coupler[j], dc, xx, yy, right);
            for (j = 0; j < organfile->manual[i].NumberOfDivisionals; j++)
                HelpDrawButton(&organfile->manual[i].divisional[j], dc, xx, yy, right);
            if (dc || !organfile->manual[i].Displayed)
                continue;
            wxRect rect(organfile->manual[i].m_X, organfile->manual[i].m_Y, organfile->manual[i].m_Width, organfile->manual[i].m_Height);
            if (rect.Contains(xx, yy))
            {
                if (right)
                    organfile->manual[i].MIDI();
                else
                    ;
            }
        }
        for (j = 0; j < organfile->NumberOfTremulants; j++)
            HelpDrawStop(&organfile->tremulant[j], dc, xx, yy, right);
        for (j = 0; j < organfile->NumberOfDivisionalCouplers; j++)
            HelpDrawStop(&organfile->divisionalcoupler[j], dc, xx, yy, right);
        for (j = 0; j < organfile->NumberOfGenerals; j++)
            HelpDrawButton(&organfile->general[j], dc, xx, yy, right);
        for (j = 0; j < organfile->NumberOfReversiblePistons; j++)
            HelpDrawButton(&organfile->piston[j], dc, xx, yy, right);
    }

	for (i = 0; i < organfile->NumberOfEnclosures; i++)
	{
        if (organfile->enclosure[i].Draw(xx, yy, dc))
        {
            if (right)
                organfile->enclosure[i].MIDI();
            else if (scroll)
                organfile->enclosure[i].Set(organfile->enclosure[i].MIDIValue + (scroll > 0 ? 16 : -16));
        }
	}
}

void OrganView::WrapText(wxDC& dc, wxString& string, int width)
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
