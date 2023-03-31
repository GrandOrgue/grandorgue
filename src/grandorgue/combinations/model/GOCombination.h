/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOCOMBINATION_H
#define GOCOMBINATION_H

#include <unordered_set>
#include <vector>

#include "yaml/GOSaveableToYaml.h"

#include "config/GOConfigReader.h"

#include "GOCombinationDefinition.h"
#include "GOSaveableObject.h"

class GOOrganController;

class GOCombination : public GOSaveableObject, public GOSaveableToYaml {
public:
  enum SetterType { SETTER_REGULAR, SETTER_SCOPE, SETTER_SCOPED };
  using ExtraElementsSet = std::unordered_set<unsigned>;

private:
  const GOCombinationDefinition &m_Template;
  GOOrganController *m_OrganFile;
  std::vector<int> m_State;

protected:
  const std::vector<GOCombinationDefinition::Element> &r_ElementDefinitions;
  bool m_Protected;

  void UpdateState();

  // Read the NumberOfStops key from the given config source
  unsigned ReadNumberOfStops(
    GOConfigReader &cfg, GOSettingType srcType, unsigned maxStops) const;
  void WriteNumberOfStops(GOConfigWriter &cfg, unsigned stopCount) const;

  static bool isCmbOnFile(GOConfigReader &cfg, const wxString &group);

  /**
   * Set the combination element state when loading. If the element is not found
   * or it's state has already been set the logs an error
   * @param manualNumber in the odf. -1 means that the element does not relate
   *   to a manual
   * @param elementType The element type
   * @param elementNumber May be positive or negative. abs(elementNumber) means
   *   the element number  among elements of the same type
   *   in the ODF (starting from 1). If elementNumber is positive then the
   *   element is  enabled in the combination. If elementNumber is negative then
   *   the element is disabled in the combination
   * @param elementName The element name for logging an error
   * @param containerName Where the element is specified. Used for logging an
   * error
   */
  void SetLoadedState(
    int manualNumber,
    GOCombinationDefinition::ElementType elementType,
    int elementNumber,
    const wxString &elementName);

  // Load the combination either from the odf or from the cmb
  virtual void LoadCombinationInt(GOConfigReader &cfg, GOSettingType srcType)
    = 0;

  virtual bool PushLocal(ExtraElementsSet const *extraSet = nullptr);

public:
  GOCombination(
    const GOCombinationDefinition &combination_template,
    GOOrganController *organController);
  virtual ~GOCombination();

  bool IsEmpty() const;
  int GetState(unsigned no) const { return m_State[no]; }
  void GetExtraSetState(ExtraElementsSet &extraSet);
  void GetEnabledElements(GOCombination::ExtraElementsSet &enabledElements);

  void Copy(GOCombination *combination);
  void Clear();

  // if present, read from CMB, else read from ODF, else clear
  void LoadCombination(GOConfigReader &cfg) override;

  /**
   * Fills the combination from the current organ elements
   */
  bool FillWithCurrent(SetterType setterType, bool isToStoreInvisibleObjects);

  /**
   * If the combination is not empty, put it into the YAML map a a key value
   * pair
   *
   * key: combination
   *
   * @param container the yaml map to put the combination to
   * @param key the key for putting this combination with
   */
  void PutToYamlMap(YAML::Node &container, const char *key) const;
  void PutToYamlMap(YAML::Node &container, const wxString &key) const {
    PutToYamlMap(container, key.mbc_str().data());
  }
  static void putToYamlMap(
    YAML::Node &container, const wxString &key, const GOCombination *pCmb);
};

#endif
