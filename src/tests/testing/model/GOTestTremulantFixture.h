/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTTREMULANTFIXTURE_H
#define GOTESTTREMULANTFIXTURE_H

#include <memory>
#include <string>

#include <wx/string.h>

#include "config/GOConfig.h"
#include "model/GOOrganModel.h"

class GOTremulant;

/**
 * What a GOTremulant needs to be constructed: a config and an empty organ
 * model. Neither does any I/O in its constructor (the same pattern as
 * GOTestEnclosure), so no ODF has to be loaded.
 */
struct GOTestTremulantModel {
  GOConfig config;
  GOOrganModel organModel;

  GOTestTremulantModel();
};

/** The group of the tremulant in the test ODF and .cmb text */
extern const wxString TEST_TREMULANT_GROUP;

/**
 * Creates a tremulant and loads it from the given ODF text and .cmb text. The
 * texts are written to temporary files and read back, because the readers work
 * with files.
 *
 * @param model the organ model of the tremulant
 * @param odfText the content of the ODF: the only group TEST_TREMULANT_GROUP,
 *   ending with a new line; the required DefaultToEngaged=N is appended to it
 * @param cmbText the content of the .cmb, may be empty
 * @return the loaded tremulant; its Load() is called with the group
 *   TEST_TREMULANT_GROUP
 */
std::unique_ptr<GOTremulant> load_tremulant_from_text(
  GOOrganModel &model, const std::string &odfText, const std::string &cmbText);

/**
 * Creates an internal tremulant, as GOTremulant::Init() does, from the given
 * .cmb text. The text is written to a temporary file and read back.
 *
 * @param model the organ model of the tremulant
 * @param cmbText the keys of the group TEST_TREMULANT_GROUP (without the group
 *   header), each ending with a new line; may be empty. The required
 *   DefaultToEngaged=N is appended to it
 * @return the initialised tremulant
 */
std::unique_ptr<GOTremulant> init_tremulant_from_cmb(
  GOOrganModel &model, const std::string &cmbText);

/**
 * Creates and loads a tremulant with the required ODF keys
 *
 * @param model the organ model of the tremulant
 * @param type the TremulantType: "Synth" or "Wave"
 * @param periodMs Period
 * @param startRate StartRate
 * @param stopRate StopRate
 * @param ampDepthPcnt AmpModDepth
 * @return the loaded tremulant
 */
std::unique_ptr<GOTremulant> load_tremulant(
  GOOrganModel &model,
  const std::string &type,
  int periodMs,
  int startRate,
  int stopRate,
  int ampDepthPcnt);

#endif /* GOTESTTREMULANTFIXTURE_H */
