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
