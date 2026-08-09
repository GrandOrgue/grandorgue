/*
 * Copyright 2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestOrganReader.h"

#include <fstream>

#include "archive/GOArchive.h"
#include "config/GOConfig.h"
#include "loader/GOFileStore.h"
#include "loader/GOOrganReader.h"
#include "loader/GOProgressMonitor.h"

#include "GOOrgan.h"
#include "go_path.h"

namespace {

class GOTestProgressMonitor : public GOProgressMonitor {
public:
  void Setup(long max, const wxString &title, const wxString &msg) override {}
  void Reset(long max, const wxString &msg) override {}
  bool Update(unsigned value, const wxString &msg) override { return true; }
};

void writeTextFile(const wxString &path, const wxString &content) {
  std::ofstream file(path.ToStdString());

  file << content.ToStdString();
}

} // namespace

GOTestOrganReader::~GOTestOrganReader() {}

std::string GOTestOrganReader::GetName() { return name; }

void GOTestOrganReader::run() {
  try {
    runImpl();
  } catch (const wxString &e) {
    throw GOTestException(
      std::string("Unexpected wxString exception: ") + e.ToStdString());
  }
}

void GOTestOrganReader::runImpl() {
  GOConfig config(GetName(), "");

  // Without a call to config.Load(), OrganSettingsPath() defaults to an
  // empty string, which would make settingsFilePath resolve under the
  // filesystem root. Redirect it into organ_directory so the "previously
  // saved per-user CMB" case below can safely write there.
  config.OrganSettingsPath(
    wxString::Format(wxT("%s/settings"), organ_directory));

  GOFileStore fileStore(config);
  GOTestProgressMonitor monitor;
  const wxString odfPath
    = wxString::Format(wxT("%s/test.organ"), organ_directory);

  writeTextFile(odfPath, wxT("[Organ]\nChurchName=Test Organ\n"));

  GOOrgan organ(odfPath);

  GOAssert(
    organ.GetODFPath() == go_normalize_path(odfPath),
    "GOOrgan should normalize the ODF path on construction");

  GOOrganReader organReader(config, organ, wxEmptyString, fileStore, monitor);
  const GOLoadedOrganInfo &info = organReader.GetLoadedOrganInfo();

  GOAssert(
    !info.isCustomized,
    "A freshly loaded ODF without a CMB should not be "
    "reported as customized");
  GOAssert(!info.odfHash.IsEmpty(), "odfHash should be filled in");
  GOAssert(
    !info.settingsFilePath.IsEmpty(), "settingsFilePath should be filled in");
  GOAssert(!info.cacheFilePath.IsEmpty(), "cacheFilePath should be filled in");

  wxString churchName = organReader.GetConfigReader().ReadString(
    ODFSetting, wxT("Organ"), wxT("ChurchName"));

  GOAssert(
    churchName == wxT("Test Organ"),
    "GetConfigReader() should read back the ChurchName written to the ODF");

  wxString churchNameFromDb;

  organReader.GetConfigDB().GetString(
    ODFSetting, wxT("Organ"), wxT("ChurchName"), churchNameFromDb);
  GOAssert(
    churchNameFromDb == churchName,
    "GetConfigDB() and GetConfigReader() should share the same underlying "
    "data");

  const wxString cmbPath
    = wxString::Format(wxT("%s/test.cmb"), organ_directory);

  writeTextFile(
    cmbPath,
    wxString::Format(
      wxT("[Organ]\nChurchName=Overridden Organ\nODFHash=%s\n"), info.odfHash));

  // cmbPath equals the ODF path with its extension replaced by ".cmb", i.e.
  // the bundled settings file GOOrganReader auto-discovers next to the ODF
  // when settingsPath is empty.
  GOOrganReader cmbOrganReader(
    config, organ, wxEmptyString, fileStore, monitor);
  const GOLoadedOrganInfo &cmbInfo = cmbOrganReader.GetLoadedOrganInfo();

  GOAssert(
    cmbInfo.isCustomized,
    "Loading with a bundled .cmb file should mark the organ as customized");

  wxString overriddenChurchName = cmbOrganReader.GetConfigReader().ReadString(
    CMBSetting, wxT("Organ"), wxT("ChurchName"));

  GOAssert(
    overriddenChurchName == wxT("Overridden Organ"),
    "The .cmb file should be layered over the ODF, not ignored");

  // An explicit settingsPath must bypass auto-discovery entirely: even
  // though a bundled .cmb (test.cmb, "Overridden Organ") already exists
  // next to the ODF, the explicitly named file must win.
  const wxString explicitCmbPath
    = wxString::Format(wxT("%s/explicit.cmb"), organ_directory);

  writeTextFile(
    explicitCmbPath,
    wxString::Format(
      wxT("[Organ]\nChurchName=Explicit Organ\nODFHash=%s\n"), info.odfHash));

  GOOrganReader explicitOrganReader(
    config, organ, explicitCmbPath, fileStore, monitor);
  const GOLoadedOrganInfo &explicitInfo
    = explicitOrganReader.GetLoadedOrganInfo();

  GOAssert(
    !explicitInfo.isCustomized,
    "Loading with an explicit settingsPath should not mark the organ as "
    "customized, matching GOOrganController's pre-refactor behaviour");

  wxString explicitChurchName
    = explicitOrganReader.GetConfigReader().ReadString(
      CMBSetting, wxT("Organ"), wxT("ChurchName"));

  GOAssert(
    explicitChurchName == wxT("Explicit Organ"),
    "An explicit settingsPath should be read directly, bypassing both the "
    "saved-settings and bundled-cmb auto-discovery checks");

  // A previously saved per-user CMB (at the computed settingsFilePath) must
  // be auto-discovered and take priority over the bundled .cmb next to the
  // ODF, even though both exist at this point.
  writeTextFile(
    info.settingsFilePath,
    wxString::Format(
      wxT("[Organ]\nChurchName=Saved Organ\nODFHash=%s\n"), info.odfHash));

  GOOrganReader savedOrganReader(
    config, organ, wxEmptyString, fileStore, monitor);
  const GOLoadedOrganInfo &savedInfo = savedOrganReader.GetLoadedOrganInfo();

  GOAssert(
    savedInfo.isCustomized,
    "Loading a previously saved per-user CMB should mark the organ as "
    "customized");

  wxString savedChurchName = savedOrganReader.GetConfigReader().ReadString(
    CMBSetting, wxT("Organ"), wxT("ChurchName"));

  GOAssert(
    savedChurchName == wxT("Saved Organ"),
    "The previously saved per-user CMB should take priority over the "
    "bundled .cmb next to the ODF");

  // A CMB with a matching ChurchName (no mismatch warning) and no ODFHash
  // entry at all (skipping the hash-mismatch check, which would otherwise
  // pop up a blocking wxMessageBox) must still be read and applied.
  const wxString matchingNoHashCmbPath
    = wxString::Format(wxT("%s/matching-no-hash.cmb"), organ_directory);

  writeTextFile(matchingNoHashCmbPath, wxT("[Organ]\nChurchName=Test Organ\n"));

  GOOrganReader matchingNoHashOrganReader(
    config, organ, matchingNoHashCmbPath, fileStore, monitor);

  wxString matchingChurchNameFromDb;
  bool wasFoundInCmb = matchingNoHashOrganReader.GetConfigDB().GetString(
    CMBSetting, wxT("Organ"), wxT("ChurchName"), matchingChurchNameFromDb);

  GOAssert(
    wasFoundInCmb && matchingChurchNameFromDb == wxT("Test Organ"),
    "A CMB with a matching ChurchName and no ODFHash entry should still be "
    "read into the CMB config, without hitting the mismatch dialogs");

  const GOOrgan missingOrgan(
    wxString::Format(wxT("%s/nonexistent.organ"), organ_directory));
  bool wasExceptionThrown = false;

  try {
    GOOrganReader missingOrganReader(
      config, missingOrgan, wxEmptyString, fileStore, monitor);
  } catch (const wxString &) {
    wasExceptionThrown = true;
  }
  GOAssert(
    wasExceptionThrown,
    "Constructing a GOOrganReader for a nonexistent ODF should throw");

  organReader.ReportUnused();
}
