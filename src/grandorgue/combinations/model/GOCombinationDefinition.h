/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOCOMBINATIONDEFINITION_H
#define GOCOMBINATIONDEFINITION_H

#include <wx/string.h>

#include <vector>

class GOCombinationElement;
class GODrawstop;
class GOManual;
class GOOrganModel;

class GOCombinationDefinition {
public:
  enum ElementType {
    COMBINATION_STOP = 0,
    COMBINATION_COUPLER = 1,
    COMBINATION_TREMULANT = 2,
    COMBINATION_DIVISIONALCOUPLER = 3,
    COMBINATION_SWITCH = 4
  };
  struct Element {
    ElementType type;
    int manual;
    unsigned index;
    bool store_unconditional;
    wxString group;
    GOCombinationElement *control;
  };

  /**
   * array of localised names of ElementTypes
   */
  static const wxString ELEMENT_TYPE_NAMES[];

private:
  GOOrganModel &r_OrganModel;
  std::vector<Element> m_elements;

  void AddGeneral(
    GODrawstop *control, ElementType type, int manual, unsigned index);
  void AddDivisional(
    GODrawstop *control, ElementType type, int manual, unsigned index);
  void Add(
    GODrawstop *control,
    ElementType type,
    int manual,
    unsigned index,
    bool store_unconditional);

public:
  GOCombinationDefinition(GOOrganModel &organModel);
  ~GOCombinationDefinition();

  GOOrganModel &GetOrganModel() { return r_OrganModel; }

  void InitGeneral();
  void InitDivisional(GOManual &manual);

  /**
   * Removes all elements. The counterpart of InitGeneral() and
   * InitDivisional(): the elements refer to the controls of the model, so it
   * must be called before they are deleted.
   */
  void Clear() { m_elements.clear(); }

  const std::vector<Element> &GetElements() const { return m_elements; };
  int FindElement(ElementType type, int manual, unsigned index) const;
};

#endif
