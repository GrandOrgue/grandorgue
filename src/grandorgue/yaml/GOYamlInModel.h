/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOYAMLINMODEL_H
#define GOYAMLINMODEL_H

#include <yaml-cpp/yaml.h>

#include "GOYamlModelBase.h"

class GOSaveableToYaml;

/**
 * Getting anything from a yaml file.
 *
 * The pattern to use:
 *
 * GOYamlInModel inYaml(organName, fileName, contentType);
 *
 * inYaml >> saveableToYaml1;
 * inYaml >> saveableToYaml2;
 */

class GOYamlInModel : private GOYamlModelBase {
private:
  YAML::Node m_GlobalNode;

public:
  /**
   * Construct an instance. If sometimes goes wrong, it throws a wxString with
   * the error messag
   * @param organName
   * @param fileName
   * @param contentType
   */
  GOYamlInModel(
    const wxString &organName,
    const wxString &fileName,
    const wxString &contentType);

  wxString GetFileOrganName() const { return geFileOrganName(m_GlobalNode); }

  const GOYamlInModel &operator>>(GOSaveableToYaml &saveableObj) const;
};

#endif /* GOYAMLINMODEL_H */
