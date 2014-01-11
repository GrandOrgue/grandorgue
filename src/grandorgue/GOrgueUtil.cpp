/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2013 GrandOrgue contributors (see AUTHORS)
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

#include "GOrgueUtil.h"

static wxChar digits[] = { wxT('0'), wxT('1'), wxT('2'), wxT('3'), wxT('4'), wxT('5'), wxT('6'), wxT('7'), wxT('8'), wxT('9') };

wxString formatCDDouble(double value)
{
	wxString result = wxEmptyString;
	unsigned vk = 1;
	if (value < 0)
	{
		result = wxT('-');
		value = -value;
	}
	while (value >= 10.0)
	{
		vk++;
		value = value / 10;
	}
	for(unsigned i = 0; i < vk; i++)
	{		
		unsigned val = (unsigned) value;
		result += digits[val];
		value = (value - val) * 10;
	}
	result += wxT(".");
	for(unsigned i = 0; i < 6; i++)
	{		
		unsigned val = (unsigned) value;
		result += digits[val];
		value = (value - val) * 10;
	}
	if (value >= 5)
	{
		int pos = result.length() - 1;
		while(pos >= 0 && (result[pos] == wxT('9') || result[pos] == wxT('.')))
		{
			if (result[pos] == wxT('9'))
				result[pos] = wxT('0');
			pos--;
		}
		if (pos < 0)
			result = wxT("1") + result;
		else if (pos == 0 && result[pos] == wxT('-'))
			result = wxT("-1") + result.Mid(1);
		else
			result[pos] = ((wxChar)result[pos]) + 1;
	}
	return result;
}

bool parseCDouble(double& result, wxString value)
{
	bool sign = false;
	unsigned pos;
	double shift = 0.1;
	result = 0;
	if (value.length() > 0 && value[0] == wxT('-'))
	{
		sign = true;
		value = value.Mid(1);
	}
	if (value.length() < 1)
		return false;
	for (pos = 0; pos < value.length(); pos++)
	{
		if (value[pos] == wxT('.'))
		{
			if (pos == 0)
				return false;
			pos++;
			break;
		}
		if (value[pos] < wxT('0') || wxT('9') < value[pos])
			return false;
		result = result * 10 + (value[pos] - wxT('0'));
	}
	for (; pos < value.length(); pos++)
	{
		if (value[pos] < wxT('0') || wxT('9') < value[pos])
			return false;
		result = result  + shift * (value[pos] - wxT('0'));
		shift = shift / 10;
	}
	if (sign)
		result = -result;
	return true;
}

bool parseLong(long& result, wxString value)
{
	bool sign = false;
	unsigned pos = 0;
	result = 0;
	if (value.length() > 0)
	{
		if (value[0] == wxT('-'))
		{
			sign = true;
			pos = 1;
		}
		else if (value[0] == wxT('+'))
			pos = 1;
	}
	if (value.length() < pos + 1)
		return false;
	for (; pos < value.length(); pos++)
	{
		if (value[pos] < wxT('0') || wxT('9') < value[pos])
			return false;
		result = result * 10 + (value[pos] - wxT('0'));
	}
	if (sign)
		result = -result;
	return true;
}

bool parseColor(wxColour& result, wxString value)
{
	if (value.length() == 7 && value[0] == wxT('#'))
	{
		unsigned r = 0, g = 0, b = 0;

		if (wxT('0') <= value[1] && value[1] <= wxT('9'))
			r = r * 10 + value[1] - wxT('0');
		else if (wxT('A') <= value[1] && value[1] <= wxT('F'))
			r = r * 10 + value[1] - wxT('A') + 10;
		else
			return false;
		if (wxT('0') <= value[2] && value[2] <= wxT('9'))
			r = r * 10 + value[2] - wxT('0');
		else if (wxT('A') <= value[2] && value[2] <= wxT('F'))
			r = r * 10 + value[2] - wxT('A') + 10;
		else
			return false;

		if (wxT('0') <= value[3] && value[3] <= wxT('9'))
			g = g * 10 + value[3] - wxT('0');
		else if (wxT('A') <= value[3] && value[3] <= wxT('F'))
			g = g * 10 + value[3] - wxT('A') + 10;
		else
			return false;
		if (wxT('0') <= value[4] && value[4] <= wxT('9'))
			g = g * 10 + value[4] - wxT('0');
		else if (wxT('A') <= value[4] && value[4] <= wxT('F'))
			g = g * 10 + value[4] - wxT('A') + 10;
		else
			return false;

		if (wxT('0') <= value[5] && value[5] <= wxT('9'))
			b = b * 10 + value[5] - wxT('0');
		else if (wxT('A') <= value[5] && value[5] <= wxT('F'))
			b = b * 10 + value[5] - wxT('A') + 10;
		else
			return false;
		if (wxT('0') <= value[6] && value[6] <= wxT('9'))
			b = b * 10 + value[6] - wxT('0');
		else if (wxT('A') <= value[6] && value[6] <= wxT('F'))
			b = b * 10 + value[6] - wxT('A') + 10;
		else
			return false;

		result.Set(r, g, b);
		return true;
	}
	return false;
}
