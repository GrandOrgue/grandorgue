/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEMIDISENDER_H
#define GORGUEMIDISENDER_H

#include "GOrgueMidiSenderData.h"

#include <wx/string.h>

class GOrgueConfigReader;
class GOrgueConfigWriter;
class GOrgueMidiMap;
class GrandOrgueFile;
struct IniFileEnumEntry;

class GOrgueMidiSender : public GOrgueMidiSenderData
{
private:
	static const struct IniFileEnumEntry m_MidiTypes[];
	GrandOrgueFile* m_organfile;
	int m_ElementID;

public:
	GOrgueMidiSender(GrandOrgueFile* organfile, MIDI_SENDER_TYPE type);
	~GOrgueMidiSender();

	void SetElementID(int id);

	void Load(GOrgueConfigReader& cfg, wxString group, GOrgueMidiMap& map);
	void Save(GOrgueConfigWriter& cfg, wxString group, GOrgueMidiMap& map);

	void SetDisplay(bool state);
	void SetKey(unsigned key, unsigned velocity);
	void ResetKey();
	void SetValue(unsigned value);
	void SetLabel(const wxString& text);
	void SetName(const wxString& text);

	bool HasChannel(midi_send_message_type type);
	bool HasKey(midi_send_message_type type);
	bool HasLowValue(midi_send_message_type type);
	bool HasHighValue(midi_send_message_type type);
	bool HasStart(midi_send_message_type type);
	bool HasLength(midi_send_message_type type);
	unsigned KeyLimit(midi_send_message_type type);
	unsigned LowValueLimit(midi_send_message_type type);
	unsigned HighValueLimit(midi_send_message_type type);
	unsigned StartLimit(midi_send_message_type type);
	unsigned LengthLimit(midi_send_message_type type);

	void Assign(const GOrgueMidiSenderData& data);
};

#endif
