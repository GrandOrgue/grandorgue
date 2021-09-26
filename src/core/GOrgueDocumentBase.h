/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEDOCUMENTBASE_H
#define GORGUEDOCUMENTBASE_H

#include <vector>

class GOrgueView;

class GOrgueDocumentBase
{
public:
	typedef enum { ORGAN_DIALOG, MIDI_EVENT, MIDI_LIST, PANEL } WindowType;
private:
	typedef struct {
		WindowType type;
		void* data;
		GOrgueView* window;
	} WindowInfo;

	std::vector<WindowInfo> m_Windows;

protected:
	void SyncState();
	void CloseWindows();

public:
	GOrgueDocumentBase();
	~GOrgueDocumentBase();

	bool WindowExists(WindowType type, void* data);
	bool showWindow(WindowType type, void* data);
	void registerWindow(WindowType type, void* data, GOrgueView *window);
	void unregisterWindow(GOrgueView* window);
};

#endif
