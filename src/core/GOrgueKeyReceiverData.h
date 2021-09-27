/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
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
