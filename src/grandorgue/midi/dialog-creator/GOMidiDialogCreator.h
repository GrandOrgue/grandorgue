/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIDIALOGCREATOR_H
#define GOMIDIDIALOGCREATOR_H

class wxString;

class GOMidiDialogListener;
class GOMidiObject;

/**
 * An abstract class that can show a MIDIEventDialog
 */
class GOMidiDialogCreator {
public:
  virtual void ShowMIDIEventDialog(
    void *element,
    const wxString &title,
    const wxString &dialogSelector,
    GOMidiObject &obj,
    GOMidiDialogListener *pDialogListener = nullptr)
    = 0;
};

#endif /* GOMIDIDIALOGCREATOR_H */
