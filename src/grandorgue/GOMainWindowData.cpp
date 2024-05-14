/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMainWindowData.h"

#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"

#include "GOOrganController.h"

void GOMainWindowData::Load(GOConfigReader &cfg) {
  p_organFile->RegisterSaveableObject(this);
  m_rect.x = cfg.ReadInteger(
    CMBSetting, m_group, wxT("WindowX"), -20, 10000, false, 0);
  m_rect.y = cfg.ReadInteger(
    CMBSetting, m_group, wxT("WindowY"), -20, 10000, false, 0);
  m_rect.width = cfg.ReadInteger(
    CMBSetting, m_group, wxT("WindowWidth"), 0, 10000, false, 0);
  m_rect.height = cfg.ReadInteger(
    CMBSetting, m_group, wxT("WindowHeight"), 0, 10000, false, 0);
}

void GOMainWindowData::Save(GOConfigWriter &cfg) {
  cfg.WriteInteger(m_group, wxT("WindowX"), std::max(m_rect.x, -20));
  cfg.WriteInteger(m_group, wxT("WindowY"), std::max(m_rect.y, -20));
  cfg.WriteInteger(m_group, wxT("WindowWidth"), m_rect.width);
  cfg.WriteInteger(m_group, wxT("WindowHeight"), m_rect.height);
}
