/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOYAMLMODELBASE_H
#define GOYAMLMODELBASE_H

#include <wx/string.h>

namespace YAML {
class Node;
}

/**
 * A common code used for writting data to and reading data from yaml
 */
class GOYamlModelBase {
protected:
  /**
   * Put the info section to the global node
   */
  static void putInfoNode(
    const wxString &contentType,
    const wxString &organName,
    YAML::Node &globalNode);

  /**
   * Get the content type of the saved yaml file
   * @param globalNode the root yaml node
   * @return the content type
   */
  static wxString getFileContentType(const YAML::Node &globalNode);

  /**
   * Get the organ name the file was saved for
   * @param globalNode the root yaml node
   * @return the organ name
   */
  static wxString geFileOrganName(const YAML::Node &globalNode);
};

#endif /* GOYAMLMODELBASE_H */
