/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDISELEMENT_H
#define GOMIDISELEMENT_H

#include "GOStringSet.h"

namespace YAML {
class Node;
};

class GOMidiMap;

class GOMidiElement {
public:
  virtual void ToYaml(YAML::Node &yamlNode, GOMidiMap &map) const = 0;
  virtual void FromYaml(
    const YAML::Node &yamlNode,
    const wxString &yamlPath,
    GOMidiMap &map,
    GOStringSet &usedPaths)
    = 0;
};

#endif /* GOMIDISELEMENT_H */
