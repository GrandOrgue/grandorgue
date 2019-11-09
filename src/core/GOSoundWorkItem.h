/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
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

#ifndef GOSOUNDWORKITEM_H
#define GOSOUNDWORKITEM_H

class GOSoundWorkItem
{
public:
	virtual ~GOSoundWorkItem()
	{
	}

	virtual unsigned GetGroup() = 0;
	virtual unsigned GetCost() = 0;
	virtual bool GetRepeat() = 0;
	virtual void Run() = 0;
	virtual void Exec() = 0;

	virtual void Clear() = 0;
	virtual void Reset() = 0;

	enum {
		TREMULANT = 10,
		WINDCHEST = 20,
		AUDIOGROUP = 50,
		AUDIOOUTPUT = 100,
		AUDIORECORDER = 150,
		RELEASE = 160,
		TOUCH = 700,
	};
};

#endif
