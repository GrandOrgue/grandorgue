/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUERANK_H
#define GORGUERANK_H

#include "ptrvector.h"
#include "GOrgueMidiConfigurator.h"
#include "GOrguePipeConfigTreeNode.h"
#include "GOrgueMidiSender.h"
#include "GOrguePlaybackStateHandler.h"
#include "GOrgueSaveableObject.h"

class GOrguePipe;
class GOrgueStop;
class GOrgueTemperament;
class GrandOrgueFile;

class GOrgueRank : private GOrgueSaveableObject, public GOrgueMidiConfigurator, private GOrguePlaybackStateHandler
{
private:
	GrandOrgueFile* m_organfile;
	wxString m_Name;
	ptr_vector<GOrguePipe> m_Pipes;
	unsigned m_StopCount;
	std::vector<unsigned> m_Velocity;
	std::vector<std::vector<unsigned> > m_Velocities;
	unsigned m_FirstMidiNoteNumber;
	bool m_Percussive;
	unsigned m_WindchestGroup;
	unsigned m_HarmonicNumber;
	float m_PitchCorrection;
	float m_MinVolume;
	float m_MaxVolume;
	bool m_RetuneRank;
	GOrgueMidiSender m_sender;
	GOrguePipeConfigTreeNode m_PipeConfig;

	void Resize();

	void Save(GOrgueConfigWriter& cfg);

	void AbortPlayback();
	void PreparePlayback();
	void StartPlayback();
	void PrepareRecording();

public:
	GOrgueRank(GrandOrgueFile* organfile);
	~GOrgueRank();
	void Init(GOrgueConfigReader& cfg, wxString group, wxString name, int first_midi_note_number, unsigned windchest);
	void Load(GOrgueConfigReader& cfg, wxString group, int first_midi_note_number);
	void AddPipe(GOrguePipe* pipe);
	unsigned RegisterStop(GOrgueStop* stop);
	void SetKey(int note, unsigned velocity, unsigned stopID);
	GOrguePipe* GetPipe(unsigned index);
	unsigned GetPipeCount();
	GOrguePipeConfigNode& GetPipeConfig();
	void SetTemperament(const GOrgueTemperament& temperament);
	const wxString& GetName();

	wxString GetMidiType();
	wxString GetMidiName();
	void ShowConfigDialog();

	wxString GetElementStatus();
	std::vector<wxString> GetElementActions();
	void TriggerElementActions(unsigned no);

	void SendKey(unsigned note, unsigned velocity);
};

#endif
