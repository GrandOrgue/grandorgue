/*
 * Copyright 2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestOrganController.h"

#include <set>
#include <string>
#include <vector>

#include "config/GOConfig.h"
#include "config/GOConfigFileWriter.h"
#include "config/GOConfigWriter.h"
#include "control/GOButtonControl.h"
#include "loader/GOOrganReader.h"
#include "loader/GOProgressMonitor.h"
#include "model/GOEnclosure.h"
#include "model/GOWindchest.h"

#include "GOBuffer.h"
#include "GOOrgan.h"
#include "GOOrganController.h"
#include "GOTestScope.h"

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
    GO_RUN_TEST(TestOnStateLifecycleSending())
    GO_RUN_TEST(TestOrganCoreData())
  } catch (const wxString &e) {
    throw GOTestException(
      std::string("Unexpected wxString exception: ") + e.ToStdString());
  }
}

void GOTestOrganController::TestOnStateLifecycleSending() {
  GOAssert(
    controller->p_OnStateButton,
    "The On indicator button should exist after the constructor");
  GOAssert(
    !controller->p_OnStateButton->IsLifecycleSendingEnabled(),
    "The On indicator should not send MIDI events in the lifecycle calls");
}

// Deliberately does not go through the public Load()/Clear(), since those
// unconditionally call LoadObjects() (whose error path pops up a
// GOMessageBox, needing a display, when GOMetronome's sound files aren't
// installed in this environment). GOTestOrganController is a friend of
// GOOrganController precisely so this test can call the non-cache phase
// methods (LoadOrganCoreData/SaveOrganCoreData/ClearOrganCoreData)
// directly, skipping the other phases entirely.
void GOTestOrganController::TestOrganCoreData() {
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
  // 1 windchest group from the ODF (NumberOfWindchestGroups=1) plus 1 more
  // that GOMetronome::createWindchest() creates to route its own sound and
  // LoadOrganCoreData() registers, regardless of the ODF's own count.
  GOAssert(
    controller->GetWindchestCount() == 2,
    "LoadOrganCoreData() should populate the ODF's windchest group plus "
    "GOMetronome's own, got: "
      + std::to_string(controller->GetWindchestCount()));

  // The internal enclosures: the master and one per windchest (including the
  // metronome's), created by the controller and not by the GUI panel.
  const unsigned nWindchests = controller->GetWindchestCount();

  GOAssert(
    controller->GetEnclosureCount() == 1 + nWindchests,
    "InitInternalEnclosures() should create the master and one enclosure per "
    "windchest, got: "
      + std::to_string(controller->GetEnclosureCount()));
  GOAssert(
    controller->GetODFEnclosureCount() == 0,
    "The internal enclosures should not be counted as ODF enclosures, got: "
      + std::to_string(controller->GetODFEnclosureCount()));
  GOAssert(
    controller->GetMasterEnclosure()
      && controller->GetMasterEnclosure()->GetName() == wxT("Master")
      && controller->GetEnclosureElement(0) == controller->GetMasterEnclosure(),
    "The master enclosure should be the first enclosure");
  for (unsigned windchestI = 0; windchestI < nWindchests; windchestI++) {
    GOEnclosure *pEnc = controller->GetInternalEnclosure(windchestI);
    const std::vector<GOEnclosure *> &windchestEncs
      = controller->GetWindchest(windchestI)->GetEnclosures();
    const std::string suffix
      = ", windchest index: " + std::to_string(windchestI);

    GOAssert(
      pEnc == controller->GetEnclosureElement(windchestI + 1),
      "Internal enclosures should follow the master in the windchest "
      "order"
        + suffix);
    GOAssert(
      pEnc->GetName() == controller->GetWindchest(windchestI)->GetName(),
      "An internal enclosure should be named after its windchest" + suffix);
    GOAssert(
      windchestEncs.size() == 2
        && windchestEncs[0] == controller->GetMasterEnclosure()
        && windchestEncs[1] == pEnc,
      "A windchest should have the master and its own enclosure" + suffix);
  }
  GOAssert(
    controller->GetInternalEnclosure(0)->GetHardName() == wxT("Windchest001"),
    "An ODF windchest without a hard name should give Windchest001");
  GOAssert(
    controller->GetInternalEnclosure(nWindchests - 1)->GetHardName()
      == wxT("Metronome"),
    "The metronome windchest should have an internal enclosure with its hard "
    "name");

  // Regression test: GOOrganModel::Load() must register every ODF windchest
  // in m_WindchestNByPipeConfig (via AddWindchest()), not just push it onto
  // m_windchests directly. GOCommonControllerTest-based fixtures always
  // build windchests through AddWindchest() themselves, so only a real
  // Load() call - as done here - can catch a regression where the ODF
  // windchest loop bypasses it and CollectWindchestsForNode() then fails
  // its assert(windchestN) for every real ODF windchest.
  for (unsigned windchestI = 0; windchestI < controller->GetWindchestCount();
       windchestI++) {
    std::set<unsigned> outWindchests;

    controller->CollectWindchestsForNode(
      controller->GetWindchest(windchestI)->GetPipeConfig(), outWindchests);
    GOAssert(
      outWindchests == std::set<unsigned>{windchestI + 1},
      "CollectWindchestsForNode() should resolve a real ODF-loaded "
      "windchest's own node to its windchest number, windchest index: "
        + std::to_string(windchestI));
  }

  // SaveOrganCoreData() round-trip.
  GOConfigFileWriter cfgFile;
  GOConfigWriter cfgWriter(cfgFile, false);

  controller->SaveOrganCoreData(cfgWriter);

  const wxString cmbPath
    = wxString::Format(wxT("%s/test.cmb"), organ_directory);

  GOAssert(
    cfgFile.Save(cmbPath), "SaveOrganCoreData() output should be writable");
  GOAssert(wxFileExists(cmbPath), "The .cmb file should have been created");

  {
    // the file is compressed, so take the plain content from the writer
    GOBuffer<uint8_t> cmbBuf;

    GOAssert(
      cfgFile.GetFileContent(cmbBuf, false),
      "The plain .cmb content should be available");

    const std::string cmbText((const char *)cmbBuf.get(), cmbBuf.GetSize());

    for (const char *pGroup :
         {"[SetterMasterVolume]", "[SetterMaster001]", "[SetterMaster002]"})
      GOAssert(
        cmbText.find(pGroup) != std::string::npos,
        std::string("The saved .cmb should contain ") + pGroup);
  }

  // ClearOrganCoreData() idempotency: calling it multiple times must not
  // crash, and a second call must be a safe no-op (this is exactly what
  // the m_IsOrganCoreDataLoaded flag is for).
  controller->ClearOrganCoreData();
  GOAssert(
    controller->GetWindchestCount() == 0,
    "ClearOrganCoreData() should remove the windchest group");
  GOAssert(
    !controller->GetMasterEnclosure(),
    "ClearOrganCoreData() should reset the master enclosure");
  controller->ClearOrganCoreData();
  GOAssert(
    controller->GetWindchestCount() == 0,
    "A second ClearOrganCoreData() call should be a safe no-op");
}
