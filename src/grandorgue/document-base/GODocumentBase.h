/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GODOCUMENTBASE_H
#define GODOCUMENTBASE_H

#include <vector>

class GOView;

class GODocumentBase {
public:
  typedef enum { ORGAN_DIALOG, MIDI_EVENT, MIDI_LIST, PANEL } WindowType;

private:
  typedef struct {
    WindowType type;
    void *data;
    GOView *window;
  } WindowInfo;

  std::vector<WindowInfo> m_Windows;

protected:
  void SyncState();
  void CloseWindows();

public:
  GODocumentBase();
  ~GODocumentBase();

  bool WindowExists(WindowType type, void *data);
  bool showWindow(WindowType type, void *data);
  void registerWindow(WindowType type, void *data, GOView *window);
  void unregisterWindow(GOView *window);
};

#endif
