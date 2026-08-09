/*
 * Copyright 2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestDivisionalSetter.h"

#include <filesystem>
#include <memory>

#include "combinations/GODivisionalSetter.h"
#include "combinations/GOSetter.h"
#include "config/GOConfigFileReader.h"
#include "config/GOConfigReader.h"
#include "config/GOConfigReaderDB.h"
#include "control/GOButtonControl.h"
#include "model/GOSwitch.h"

#include "GOOrganController.h"

// Loads the static ODF fixture fixtureName (from the fixtures/ directory next
// to this file) into the controller, then constructs and loads a
// GODivisionalSetter for it.
//
// This does not call GOOrganController::Load(): that unconditionally builds
// real GUI panels (GOGUIPanel::Load), which needs a live GTK display that
// GOTestExe does not have. Instead this replicates the slice of
// GOOrganController::ReadOrganFile that builds the non-GUI model and the
// setter/divisional-setter (SetCombinationController -> GOOrganModel::Load
// -> LoadCmbButtons -> construct GODivisionalSetter -> Load() on both the
// setter and the divisional setter) and skips the GUI-panel block. If
// ReadOrganFile's ordering ever changes, this needs to be revisited. The
// returned GODivisionalSetter must outlive any button lookups/pushes done
// against it, since its buttons are registered against the controller.
static std::unique_ptr<GODivisionalSetter> load_test_organ(
  GOOrganController &controller,
  GOTestUtils &testUtils,
  const std::string &fixtureName) {
  const std::filesystem::path fixturePath
    = std::filesystem::path(GOTEST_FIXTURES_DIR) / fixtureName;
  GOConfigFileReader odfIniFile;

  odfIniFile.Read(fixturePath.string());

  GOConfigReaderDB ini(controller.GetConfig().ODFCheck());

  ini.ReadData(odfIniFile, ODFSetting, false);

  GOConfigReader cfg(
    ini,
    controller.GetConfig().ODFCheck(),
    controller.GetConfig().ODFHw1Check());

  controller.SetCombinationController(controller.GetSetter());
  // GOOrganController::Load(const GOOrgan&, ...) can't be used here: it
  // unconditionally builds real GUI panels (GOGUIPanel::Load), which need a
  // live GTK display GOTestExe doesn't have. It also hides the inherited
  // GOOrganModel::Load(GOConfigReader&) we want instead, hence the
  // qualified call below.
  controller.GOOrganModel::Load(cfg);
  controller.LoadCmbButtons(cfg);

  auto pDivSetter = std::make_unique<GODivisionalSetter>(
    &controller, controller.GetSetter()->GetState());

  controller.GetSetter()->Load(cfg);
  pDivSetter->Load(cfg);

  testUtils.GOAssert(
    bool(pDivSetter), "Failed to build the GODivisionalSetter");

  return pDivSetter;
}

// Looks up buttons directly on the GOSetter/GODivisionalSetter element
// creators rather than via GOOrganController::GetButtonControl(), because
// the latter only searches m_elementcreators, which load_test_organ never
// populates (that only happens inside the GUI-panel-building
// GOOrganController::ReadOrganFile, which this test deliberately bypasses).
static GOButtonControl *get_divisional_button(
  GODivisionalSetter &divSetter, unsigned manualIndex) {
  return divSetter.GetButtonControl(
    GODivisionalSetter::GetDivisionalButtonName(manualIndex, 0), false);
}

// Presses Set then a banked divisional piston (bank A, position 1) for the
// manual at manualIndex, simulating a user pressing Set followed by a
// divisional piston. The Set button's lookup name is "Set" (BUTTON_DEFS in
// GOSetter.cpp) -- "SetterSet" is only its config *group* name, passed to
// Init(cfg, wxT("SetterSet"), ...), and is a different string.
static void set_divisional(
  GOSetter &setter, GODivisionalSetter &divSetter, unsigned manualIndex) {
  GOButtonControl *const pSet = setter.GetButtonControl(wxT("Set"), false);
  GOButtonControl *const pDivisional
    = get_divisional_button(divSetter, manualIndex);

  pSet->Push();
  pDivisional->Push();
  pSet->Push();
}

std::string GOTestDivisionalSetter::CLASS_NAME = "GOTestDivisionalSetter";

std::string GOTestDivisionalSetter::TestWithoutPedal::TEST_NAME
  = CLASS_NAME + "::TestWithoutPedal";

void GOTestDivisionalSetter::TestWithoutPedal::run() {
  // Reproduces https://github.com/GrandOrgue/grandorgue/discussions/2552:
  // on an organ without a Pedal, GetFirstManualIndex() == 1, which used to
  // make GODivisionalSetter::SwitchDivisionalTo misidentify the pressed
  // manual as a coupled one, so Set never stored anything.
  const std::unique_ptr<GODivisionalSetter> pDivSetter
    = load_test_organ(*controller, *this, "NoPedal.organ");

  const unsigned manualIndex = 1;
  GOSwitch *const pSwitch = controller->GetSwitch(0);
  GOButtonControl *const pDivisional
    = get_divisional_button(*pDivSetter, manualIndex);

  GOAssert(!pSwitch->IsEngaged(), "The switch should be off initially");
  GOAssert(
    !pDivisional->IsEngaged(), "The divisional should not be lit initially");

  pSwitch->SetButtonState(true);
  set_divisional(*controller->GetSetter(), *pDivSetter, manualIndex);

  GOAssert(
    pDivisional->IsEngaged(),
    "The divisional should be lit after Set on a no-pedal organ");

  // Now check that it can be recalled: switch off, press the divisional
  // again without Set active, and the switch should turn back on.
  pSwitch->SetButtonState(false);

  GOButtonControl *const pDivisional2
    = get_divisional_button(*pDivSetter, manualIndex);

  pDivisional2->Push();

  GOAssert(
    pSwitch->IsEngaged(),
    "Recalling the divisional should re-engage the switch");
}

std::string GOTestDivisionalSetter::TestWithPedal::TEST_NAME
  = CLASS_NAME + "::TestWithPedal";

void GOTestDivisionalSetter::TestWithPedal::run() {
  // Same scenario as TestWithoutPedal but with a Pedal present
  // (GetFirstManualIndex() == 0), which must keep working as before.
  const std::unique_ptr<GODivisionalSetter> pDivSetter
    = load_test_organ(*controller, *this, "WithPedal.organ");

  const unsigned manualIndex = 1;
  GOSwitch *const pSwitch = controller->GetSwitch(0);
  GOButtonControl *const pDivisional
    = get_divisional_button(*pDivSetter, manualIndex);

  pSwitch->SetButtonState(true);
  set_divisional(*controller->GetSetter(), *pDivSetter, manualIndex);

  GOAssert(
    pDivisional->IsEngaged(),
    "The divisional should be lit after Set on a with-pedal organ");

  pSwitch->SetButtonState(false);

  GOButtonControl *const pDivisional2
    = get_divisional_button(*pDivSetter, manualIndex);

  pDivisional2->Push();

  GOAssert(
    pSwitch->IsEngaged(),
    "Recalling the divisional should re-engage the switch");
}

std::string GOTestDivisionalSetter::TestDivisionalCoupler::TEST_NAME
  = CLASS_NAME + "::TestDivisionalCoupler";

void GOTestDivisionalSetter::TestDivisionalCoupler::run() {
  // Two manuals coupled by an engaged, bidirectional DivisionalCoupler.
  // GOSetter::ProcessPushDivisional only pushes into a coupled manual's
  // *existing* combination (GODivisionalSetter.cpp:396-415: pCoupledCmb must
  // already be present in that manual's DivisionalMap) -- Set does not
  // auto-create combinations across coupled manuals, only on the manual
  // actually pressed. So each manual first gets its own combination via its
  // own Set press; the coupling is then exercised via recall (pressing a
  // divisional with Set *not* active), which -- unlike Set -- always pushes
  // to every coupled manual regardless of coupledManualIndex (GOSetter.cpp:
  // "!m_state.isSetActive || coupledManualIndex == startManualIndex" is true
  // whenever Set is inactive). This is exactly the coupled-manual loop in
  // SwitchDivisionalTo/ProcessPushDivisional that contained the bug.
  const std::unique_ptr<GODivisionalSetter> pDivSetter
    = load_test_organ(*controller, *this, "DivisionalCoupler.organ");

  const unsigned manual1Index = 1;
  const unsigned manual2Index = 2;
  GOSetter &setter = *controller->GetSetter();
  GOSwitch *const pSwitch1 = controller->GetSwitch(0);
  GOSwitch *const pSwitch2 = controller->GetSwitch(1);

  // Give each manual its own combination at the same bank/slot.
  pSwitch1->SetButtonState(true);
  set_divisional(setter, *pDivSetter, manual1Index);
  pSwitch2->SetButtonState(true);
  set_divisional(setter, *pDivSetter, manual2Index);

  GOButtonControl *const pDivisional1
    = get_divisional_button(*pDivSetter, manual1Index);
  GOButtonControl *const pDivisional2
    = get_divisional_button(*pDivSetter, manual2Index);

  GOAssert(
    pDivisional1->IsEngaged(),
    "The divisional should be lit on manual 1 after its own Set");
  GOAssert(
    pDivisional2->IsEngaged(),
    "The divisional should be lit on manual 2 after its own Set");

  // Recall (Set not active) on manual 1 alone must also recall manual 2's
  // combination through the coupler.
  pSwitch1->SetButtonState(false);
  pSwitch2->SetButtonState(false);
  pDivisional1->Push();

  GOAssert(
    pSwitch1->IsEngaged(),
    "Recalling manual 1's divisional should re-engage switch 1");
  GOAssert(
    pSwitch2->IsEngaged(),
    "Recalling manual 1's divisional should also re-engage switch 2 through "
    "the coupler");
}
