/*
 * Copyright 2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestOrganController.h"

#include "config/GOConfig.h"
#include "config/GOConfigFileWriter.h"
#include "config/GOConfigWriter.h"
#include "loader/GOOrganReader.h"
#include "loader/GOProgressMonitor.h"

#include "GOOrgan.h"
#include "GOOrganController.h"

// Smallest ODF that survives LoadOrganCoreData() to completion: one
// windchest group, one manual, no stops/couplers/tremulants/switches/ranks
// (all optional, default to 0). Checked into src/tests/testing/resources
// rather than generated at runtime.
static const wxString MINIMAL_ODF_PATH
  = wxT(GO_TEST_RESOURCES_DIR "/minimal.organ");

static class GOTestProgressMonitor : public GOProgressMonitor {
public:
  void Setup(long max, const wxString &title, const wxString &msg) override {}
  void Reset(long max, const wxString &msg) override {}
  bool Update(unsigned value, const wxString &msg) override { return true; }
} s_ProgressMonitor;

GOTestOrganController::~GOTestOrganController() {}

std::string GOTestOrganController::GetName() { return name; }

void GOTestOrganController::run() {
  try {
    runImpl();
  } catch (const wxString &e) {
    throw GOTestException(
      std::string("Unexpected wxString exception: ") + e.ToStdString());
  }
}

// Deliberately does not go through the public Load()/Clear(), since those
// unconditionally call LoadOrganGui()/ClearOrganGui() (which build real
// panels needing a live GUI display) and LoadObjects() (whose error path
// pops up a GOMessageBox, also needing a display, when GOMetronome's sound
// files aren't installed in this environment). GOTestOrganController is a
// friend of GOOrganController precisely so this test can call the
// non-GUI, non-cache phase methods (LoadOrganCoreData/SaveOrganCoreData/
// ClearOrganCoreData) directly, skipping the other phases entirely.
void GOTestOrganController::runImpl() {
  const GOOrgan organ(MINIMAL_ODF_PATH);

  GOOrganReader organReader(
    controller->m_config,
    organ,
    wxEmptyString,
    controller->m_FileStore,
    s_ProgressMonitor);

  controller->LoadOrganCoreData(organReader.GetConfigReader());
  organReader.ReportUnused();

  GOAssert(
    controller->GetOrganName() == wxT("Test Organ"),
    "LoadOrganCoreData() should populate the organ name from the ODF");
  GOAssert(
    controller->GetPanelCount() == 0,
    "LoadOrganCoreData() alone must not build any panels "
    "(that's LoadOrganGui's job, not exercised here)");
  // 1 windchest group from the ODF (NumberOfWindchestGroups=1) plus 1 more
  // that GOMetronome::Load() creates for itself via AddWindchest() to route
  // its own sound (GOMetronome.cpp), regardless of the ODF's own count.
  GOAssert(
    controller->GetWindchestCount() == 2,
    "LoadOrganCoreData() should populate the ODF's windchest group plus "
    "GOMetronome's own, got: "
      + std::to_string(controller->GetWindchestCount()));

  // SaveOrganCoreData() round-trip.
  GOConfigFileWriter cfgFile;
  GOConfigWriter cfgWriter(cfgFile, false);

  controller->SaveOrganCoreData(cfgWriter);

  const wxString cmbPath
    = wxString::Format(wxT("%s/test.cmb"), organ_directory);

  GOAssert(
    cfgFile.Save(cmbPath), "SaveOrganCoreData() output should be writable");
  GOAssert(wxFileExists(cmbPath), "The .cmb file should have been created");

  // ClearOrganCoreData() idempotency: calling it multiple times must not
  // crash, and a second call must be a safe no-op (this is exactly what
  // the m_IsOrganCoreDataLoaded flag is for).
  controller->ClearOrganCoreData();
  GOAssert(
    controller->GetWindchestCount() == 0,
    "ClearOrganCoreData() should remove the windchest group");
  controller->ClearOrganCoreData();
  GOAssert(
    controller->GetWindchestCount() == 0,
    "A second ClearOrganCoreData() call should be a safe no-op");
}
