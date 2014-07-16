/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2014 GrandOrgue contributors (see AUTHORS)
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

#ifndef GOGUILAYOUTENGINE_H
#define GOGUILAYOUTENGINE_H

#include <vector>

class GOGUIDisplayMetrics;
class GOGUIEnclosure;
class GOGUIManual;

class GOGUILayoutEngine
{
public:
	typedef struct
	{
		unsigned width;
		unsigned height;
		int y;
		int x;
		int keys_y;
		int piston_y;
	} MANUAL_RENDER_INFO;

protected:
	GOGUIDisplayMetrics& m_metrics;
	std::vector<MANUAL_RENDER_INFO> m_ManualRenderInfo;
	std::vector<GOGUIEnclosure*> m_Enclosures;
	std::vector<GOGUIManual*> m_Manuals;

	/* values computed on screen update */
	int m_HackY;
	int m_EnclosureY;
	int m_CenterY;
	int m_CenterWidth;

public:
	GOGUILayoutEngine(GOGUIDisplayMetrics& metrics);
	virtual~ GOGUILayoutEngine();

	virtual void GetDrawstopBlitPosition(const int drawstopRow, const int drawstopCol, int& blitX, int& blitY);
	virtual void GetPushbuttonBlitPosition(const int buttonRow, const int buttonCol, int& blitX, int& blitY);

	virtual unsigned GetEnclosuresWidth();
	virtual int GetEnclosureY();
	virtual int GetEnclosureX(const GOGUIEnclosure* enclosure);

	virtual int GetJambLeftRightWidth();
	virtual unsigned GetJambLeftRightHeight();
	virtual int GetJambLeftRightY();
	virtual int GetJambLeftX();
	virtual int GetJambRightX();
	virtual int GetJambTopDrawstop();
	virtual int GetJambTopPiston();
	virtual unsigned GetJambTopHeight();
	virtual unsigned GetJambTopWidth();
	virtual int GetJambTopX();
	virtual int GetJambTopY();
	virtual unsigned GetPistonTopHeight();
	virtual unsigned GetPistonWidth();
	virtual int GetPistonX();
	virtual int GetCenterWidth();
	virtual int GetCenterY();
	virtual int GetCenterX();
	virtual int GetHackY();

	virtual const MANUAL_RENDER_INFO& GetManualRenderInfo(const unsigned manual_nb) const;

	void RegisterEnclosure(GOGUIEnclosure* enclosure);
	void RegisterManual(GOGUIManual* manual);

	virtual void Update();
};

#endif
