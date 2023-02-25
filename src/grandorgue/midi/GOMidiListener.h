/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDILISTENER_H
#define GOMIDILISTENER_H

class GOMidi;
class GOMidiCallback;
class GOMidiEvent;

class GOMidiListener {
  GOMidiCallback *m_Callback;
  GOMidi *m_midi;

public:
  GOMidiListener();
  virtual ~GOMidiListener();

  void SetCallback(GOMidiCallback *callback);
  void Register(GOMidi *midi);
  void Unregister();

  void Send(const GOMidiEvent &event);
};

#endif
