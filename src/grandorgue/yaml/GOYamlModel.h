/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOYAMLMODEL_H
#define GOYAMLMODEL_H

#include <wx/string.h>
#include <yaml-cpp/yaml.h>

class GOSaveableToYaml;

/**
 * A common code used for writting data to and reading data from yaml
 */
class GOYamlModel {
public:
  /**
   * Getting anything from a yaml file.
   *
   * The pattern to use:
   *
   * GOYamlModel::In inYaml(organName, fileName, contentType);
   *
   * inYaml >> saveableToYaml1;
   * inYaml >> saveableToYaml2;
   */
  class In {
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
    In(
      const wxString &organName,
      const wxString &fileName,
      const wxString &contentType);

    wxString GetFileOrganName() const;

    const In &operator>>(GOSaveableToYaml &saveableObj) const;
  };

  /**
   * Putting anything to a yaml file.
   *
   * The pattern to use:
   *
   * GOYamlModel::Out outYaml(organName, contentType);
   *
   * outYaml << saveableToYaml1;
   * outYaml << saveableToYaml2;
   *
   * wxString errorMsg = outYaml.writeTo(fileName)
   */
  class Out {
  private:
    YAML::Node m_GlobalNode;

  public:
    Out(const wxString &organName, const wxString &contentType);

    Out &operator<<(const GOSaveableToYaml &saveableObj);

    /**
     * Write the current yaml model to the text file. If something goes wrong it
     * returns a non empty error message
     */
    wxString writeTo(const wxString &fileName);
  };
};

#endif /* GOYAMLMODEL_H */
