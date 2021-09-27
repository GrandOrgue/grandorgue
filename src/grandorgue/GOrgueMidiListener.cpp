/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOrgueMidiListener.h"

#include "GOrgueMidi.h"
#include "GOrgueMidiCallback.h"
#include "GOrgueMidiEvent.h"

GOrgueMidiListener::GOrgueMidiListener() :
	m_Callback(NULL),
	m_midi(NULL)
{
}

GOrgueMidiListener::~GOrgueMidiListener()
{
	Unregister();
}

void GOrgueMidiListener::SetCallback(GOrgueMidiCallback* callback)
{
	m_Callback = callback;
}

void GOrgueMidiListener::Register(GOrgueMidi* midi)
{
	Unregister();
	if (midi)
	{
		m_midi = midi;
		m_midi->Register(this);
	}
}

void GOrgueMidiListener::Unregister()
{
	if (m_midi)
		m_midi->Unregister(this);
	m_midi = NULL;
}

void GOrgueMidiListener::Send(const GOrgueMidiEvent& event)
{
	if (m_Callback)
		m_Callback->OnMidiEvent(event);
}

