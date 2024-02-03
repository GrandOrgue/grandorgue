/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOCOMBINATION_H
#define GOCOMBINATION_H

#include <unordered_set>
#include <vector>

#include "config/GOConfigReader.h"
#include "yaml/GOSaveableToYaml.h"

#include "GOBool3.h"

#include "GOCombinationDefinition.h"
#include "GOSaveableObject.h"
#include "GOSetterState.h"

class GOOrganModel;

class GOCombination : public GOSaveableObject, public GOSaveableToYaml {
public:
  using ExtraElementsSet = std::unordered_set<unsigned>;

protected:
  GOOrganModel &r_OrganModel;

private:
  const GOCombinationDefinition &m_Template;

  /**
   *  States of the elements.
   *  BOOL3_TRUE - enabled
   *  BOOL3_FALSE - disabled
   *  BOOL3_DEFAULT - not to touch. Usually this state is used for invisible
   *    elements that are not stored in the combination normally.
   */
  std::vector<GOBool3> m_ElementStates;

  /**
   * Whether the combination has been captured when `Full` was engaged or not
   * If the combination is Full then all elements can have states 0 and 1, else
   * the state -1 is also valid.
   */
  bool m_IsFull;

  /**
   * Whether the combination has been captured when `Scope` or `Scoped` engaged
   */
  bool m_HasScope;

  void PutElementsToYaml(YAML::Node &yamlMap, GOBool3 stateFrom) const;

protected:
  const std::vector<GOCombinationDefinition::Element> &r_ElementDefinitions;
  bool m_Protected;

  void EnsureElementStatesAllocated();

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

  // Load the implementation-specific part of the combination
  virtual void LoadCombinationInt(GOConfigReader &cfg, GOSettingType srcType)
    = 0;
  // Load the combination either from the odf or from the cmb
  void LoadCombination(GOConfigReader &cfg, GOSettingType srcType);
  // Save implementation-specific part of the combination to the cmb
  virtual void SaveInt(GOConfigWriter &cfg) = 0;

  /**
   * Encode one combination element to yaml map. Called internally from ToYaml()
   * only for elements with enabled state
   */
  virtual void PutElementToYamlMap(
    const GOCombinationDefinition::Element &e,
    const wxString &valueLabel,
    const unsigned objectIndex,
    YAML::Node &yamlMap) const = 0;

  /**
   * Set states of all elements of a specified type from the yaml node. The yaml
   *   node must be a map (Number: Name). The elements may be matched by names
   *   as well by numbers. The name has a higher priority than the number.
   *   If only of them is matched then log a warning. If no
   *   of them is matched then log an error.
   * @param yamlNode the source yaml node with number-name pairs
   * @param manualNumber in the odf. -1 means that the element does not relate
   *   to any manual
   * @param elementType The element type
   */
  void SetStatesFromYaml(
    const YAML::Node &yamlNode,
    int manualNumber,
    GOCombinationDefinition::ElementType elementType);

  /**
   * Fill up the combination from yaml map. It is called internally from
   * FromYaml after cleaning the combination only when yamlMap is not empty
   * @param yamlMap
   */
  virtual void FromYamlMap(const YAML::Node &yamlMap) = 0;

public:
  GOCombination(
    GOOrganModel &organModel, const GOCombinationDefinition &cmbDef);
  virtual ~GOCombination();

  bool IsEmpty() const;
  GOBool3 GetElementState(unsigned no) const { return m_ElementStates[no]; }
  void GetExtraSetState(ExtraElementsSet &extraSet);
  void GetEnabledElements(GOCombination::ExtraElementsSet &enabledElements);

  void Copy(const GOCombination *combination);
  void Clear();

  // if present, read from CMB, else read from ODF, else clear
  void LoadCombination(GOConfigReader &cfg) override;
  // Save the combination to the cmb
  void Save(GOConfigWriter &cfg) override;

  /**
   * Fills the combination from the current organ elements
   */
  bool FillWithCurrent(
    GOSetterState::SetterType setterType, bool isToStoreInvisibleObjects);

  void ToYaml(YAML::Node &yamlMap) const override;

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

  void FromYaml(const YAML::Node &yamlNode) override;

  bool Push(
    const GOSetterState &setterState,
    const ExtraElementsSet *extraSet = nullptr);
};

#endif
