/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2017 GrandOrgue contributors (see AUTHORS)
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "GOrgueBitmap.h"

#include <wx/bitmap.h>
#include <wx/dcmemory.h>
#include <wx/image.h>

GOrgueBitmap::GOrgueBitmap() :
	m_img(NULL),
	m_Scale(0),
	m_ResultWidth(0),
	m_ResultHeight(0),
	m_ResultXOffset(0),
	m_ResultYOffset(0)
{
}

GOrgueBitmap::GOrgueBitmap(wxImage* img) :
	m_img(img),
	m_Scale(0),
	m_ResultWidth(0),
	m_ResultHeight(0),
	m_ResultXOffset(0),
	m_ResultYOffset(0)
{
}

void GOrgueBitmap::ScaleBMP(wxImage& img, double scale, const wxRect& rect, GOrgueBitmap* background)
{
	if (background && img.HasAlpha())
	{
		wxBitmap bmp(img.GetWidth(), img.GetHeight());
		wxBitmap orig(img);
		wxMemoryDC dc;

		dc.SelectObject(bmp);
		dc.DrawBitmap(background->GetBitmap(), -rect.GetX(), -rect.GetY(), false);
		dc.DrawBitmap(orig, 0, 0, true);
		bmp.SetMask(orig.GetMask());
		wxImage img_result = bmp.ConvertToImage();
		img_result.InitAlpha();
		memcpy(img_result.GetAlpha(), img.GetAlpha(), img.GetWidth() * img.GetHeight());

		m_bmp = (wxBitmap)img_result.Scale(img.GetWidth() * scale, img.GetHeight() * scale, wxIMAGE_QUALITY_BICUBIC);
	}
	else
		m_bmp = (wxBitmap)img.Scale(img.GetWidth() * scale, img.GetHeight() * scale, wxIMAGE_QUALITY_BICUBIC);
	m_Scale = scale;
}

void GOrgueBitmap::PrepareBitmap(double scale, const wxRect& rect, GOrgueBitmap* background)
{
	if (scale != m_Scale || m_ResultWidth || m_ResultHeight)
	{
		ScaleBMP(*m_img, scale, rect, background);
		m_ResultWidth = 0;
		m_ResultHeight = 0;
	}
}

void GOrgueBitmap::PrepareTileBitmap(double scale, const wxRect& rect, unsigned xo, unsigned yo, GOrgueBitmap* background)
{
	if (scale != m_Scale || m_ResultWidth != rect.GetWidth() || m_ResultHeight != rect.GetHeight() || xo != m_ResultXOffset || yo != m_ResultYOffset)
	{
		wxImage img(rect.GetWidth(), rect.GetHeight());
		for(int y = -yo; y < img.GetHeight(); y+= GetHeight())
			for(int x = -xo; x < img.GetWidth(); x+= GetWidth())
				img.Paste(*m_img, x, y);
		ScaleBMP(img, scale, rect, background);
		m_ResultWidth = rect.GetWidth();
		m_ResultHeight = rect.GetHeight();
		m_ResultXOffset = xo;
		m_ResultYOffset = yo;
	}
}

const wxBitmap& GOrgueBitmap::GetBitmap()
{
	return m_bmp;
}

unsigned GOrgueBitmap::GetWidth()
{
	return m_img->GetWidth();
}

unsigned GOrgueBitmap::GetHeight()
{
	return m_img->GetHeight();
}
