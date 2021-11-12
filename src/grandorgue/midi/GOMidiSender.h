/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOMIDISENDER_H
#define GOMIDISENDER_H

#include "midi/GOMidiSenderData.h"

#include <wx/string.h>

class GOConfigReader;
class GOConfigWriter;
class GOMidiMap;
class GODefinitionFile;
struct IniFileEnumEntry;

class GOMidiSender : public GOMidiSenderData
{
private:
	static const struct IniFileEnumEntry m_MidiTypes[];
	GODefinitionFile* m_organfile;
	int m_ElementID;

public:
	GOMidiSender(GODefinitionFile* organfile, MIDI_SENDER_TYPE type);
	~GOMidiSender();

	void SetElementID(int id);

	void Load(GOConfigReader& cfg, wxString group, GOMidiMap& map);
	void Save(GOConfigWriter& cfg, wxString group, GOMidiMap& map);

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

	void Assign(const GOMidiSenderData& data);
};

#endif
