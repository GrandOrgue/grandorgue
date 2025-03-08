/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOYAMLOUTFILE_H
#define GOYAMLOUTFILE_H

#include <yaml-cpp/yaml.h>

#include "GOYamlModelBase.h"

class GOSaveableToYaml;

/**
 * Putting anything to a yaml file.
 *
 * The pattern to use:
 *
 * GOYamlOutModel outYaml(organName, contentType);
 *
 * outYaml << saveableToYaml1;
 * outYaml << saveableToYaml2;
 *
 * wxString errorMsg = outYaml.writeTo(fileName)
 */

class GOYamlOutModel : private GOYamlModelBase {
private:
  YAML::Node m_GlobalNode;

public:
  GOYamlOutModel(const wxString &organName, const wxString &contentType);

  GOYamlOutModel &operator<<(const GOSaveableToYaml &saveableObj);

  /**
   * Write the current yaml model to the text file. If something goes wrong it
   * returns a non empty error message
   */
  wxString writeTo(const wxString &fileName);
};

#endif /* GOYAMLOUTFILE_H */
