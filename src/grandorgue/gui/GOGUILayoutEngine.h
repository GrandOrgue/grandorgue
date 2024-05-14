/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUILAYOUTENGINE_H
#define GOGUILAYOUTENGINE_H

#include <vector>

class GOGUIDisplayMetrics;
class GOGUIEnclosure;
class GOGUIManual;

class GOGUILayoutEngine {
public:
  typedef struct {
    unsigned width;
    unsigned height;
    int y;
    int x;
    int keys_y;
    int piston_y;
  } MANUAL_RENDER_INFO;

protected:
  GOGUIDisplayMetrics &m_metrics;
  std::vector<MANUAL_RENDER_INFO> m_ManualRenderInfo;
  std::vector<GOGUIEnclosure *> m_Enclosures;
  std::vector<GOGUIManual *> m_Manuals;

  /* values computed on screen update */
  int m_HackY;
  int m_EnclosureY;
  int m_CenterY;
  int m_CenterWidth;

public:
  GOGUILayoutEngine(GOGUIDisplayMetrics &metrics);
  virtual ~GOGUILayoutEngine();

  virtual void GetDrawstopBlitPosition(
    const int drawstopRow, const int drawstopCol, int &blitX, int &blitY);
  virtual void GetPushbuttonBlitPosition(
    const int buttonRow, const int buttonCol, int &blitX, int &blitY);

  virtual unsigned GetEnclosuresWidth();
  virtual int GetEnclosureY();
  virtual int GetEnclosureX(const GOGUIEnclosure *enclosure);

  virtual int GetJambLeftRightWidth();
  virtual unsigned GetJambLeftRightHeight();
  virtual int GetJambLeftRightY();
  virtual int GetJambLeftX();
  virtual int GetJambRightX();
  virtual int GetJambTopDrawstop();
  virtual int GetJambTopPiston();
  virtual unsigned GetJambTopHeight();
  virtual unsigned GetJambTopWidth();
  virtual int GetJambTopX();
  virtual int GetJambTopY();
  virtual unsigned GetPistonTopHeight();
  virtual unsigned GetPistonWidth();
  virtual int GetPistonX();
  virtual int GetCenterWidth();
  virtual int GetCenterY();
  virtual int GetCenterX();
  virtual int GetHackY();

  virtual const MANUAL_RENDER_INFO &GetManualRenderInfo(
    const unsigned manual_nb) const;

  void RegisterEnclosure(GOGUIEnclosure *enclosure);
  void RegisterManual(GOGUIManual *manual);
  unsigned GetManualNumber();

  virtual void Update();
};

#endif
