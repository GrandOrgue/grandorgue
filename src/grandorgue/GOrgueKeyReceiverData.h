/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2016 GrandOrgue contributors (see AUTHORS)
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

#ifndef GORGUEKEYRECEIVERDATA_H
#define GORGUEKEYRECEIVERDATA_H

typedef enum {
	KEY_RECV_BUTTON,
	KEY_RECV_ENCLOSURE,
} KEY_RECEIVER_TYPE;

typedef enum {
	KEY_MATCH_NONE,
	KEY_MATCH,
	KEY_MATCH_MINUS,
} KEY_MATCH_TYPE;

class GOrgueKeyReceiverData {
protected:
	KEY_RECEIVER_TYPE m_type;
	unsigned m_ShortcutKey;
	unsigned m_MinusKey;

public:
	GOrgueKeyReceiverData(KEY_RECEIVER_TYPE type);
	virtual ~GOrgueKeyReceiverData();

	KEY_RECEIVER_TYPE GetType() const;

	unsigned GetShortcut();
	void SetShortcut(unsigned key);
	unsigned GetMinusKey();
	void SetMinusKey(unsigned key);
};

#endif
