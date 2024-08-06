/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOAudioDeviceNode.h"

#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"

static const wxString WX_NAME = wxT("Name");
static const wxString WX_LATENCY = wxT("Latency");

void GOAudioDeviceNode::LoadDeviceNode(
  GOConfigReader &cfg, const wxString &group, const wxString &prefix) {
  LoadNamePattern(cfg, group, prefix, WX_NAME);
  m_DesiredLatency = cfg.ReadInteger(
    CMBSetting, group, prefix + WX_LATENCY, 0, 999, false, DEFAULT_LATENCY);
}

void GOAudioDeviceNode::SaveDeviceNode(
  GOConfigWriter &cfg, const wxString &group, const wxString &prefix) const {
  SaveNamePattern(cfg, group, prefix, WX_NAME);
  cfg.WriteInteger(group, prefix + WX_LATENCY, m_DesiredLatency);
}
