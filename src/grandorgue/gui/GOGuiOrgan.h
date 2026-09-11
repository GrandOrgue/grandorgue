/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUIORGAN_H
#define GOGUIORGAN_H

#include <wx/string.h>

#include "ptrvector.h"

#include "GOOrganController.h"
#include "document-base/GODocumentBase.h"
#include "midi/GOMidiListener.h"
#include "midi/events/GOMidiCallback.h"
#include "threading/GOMutex.h"

class GOConfig;
class GOConfigReader;
class GOGuiImageCache;
class GOGUIMouseState;
class GOGUIPanel;
class GOGUIPanelCreator;
class GOMidiDialogListener;
class GOMidiObject;
class GOMidiSystem;
class GOOrgan;
class GOProgressMonitor;
class GOResizable;

class GOGuiOrgan : public GODocumentBase, protected GOMidiCallback {
private:
  /** GUI-aware GOOrganController used only while this GOGuiOrgan has an
   * organ loaded. Its OnLoad()/OnClear() overrides run the panel-creator
   * orchestration that used to live in GOOrganController::OnLoad()/
   * OnClear() directly - the rest of that GUI data (image cache, panels,
   * main-window data, stop-window size) is still owned by
   * GOOrganController itself for now (see LoadOrganGuiData()/
   * ClearOrganGuiData() there) and reached here through m_OrganController's
   * existing accessors, until a later step moves it here too. */
  class Controller : public GOOrganController {
  private:
    GOGuiOrgan &r_GuiOrgan;

  protected:
    void OnLoad(GOConfigReader &cfg) override {
      LoadOrganGuiData(cfg);
      r_GuiOrgan.LoadOrganGui(cfg);
    }
    void OnClear() override {
      r_GuiOrgan.ClearOrganGui();
      ClearOrganGuiData();
    }

  public:
    /**
     * @param config the application config, forwarded to GOOrganController
     * @param guiOrgan the enclosing GOGuiOrgan that owns this Controller
     */
    Controller(GOConfig &config, GOGuiOrgan &guiOrgan)
      : GOOrganController(config, true), r_GuiOrgan(guiOrgan) {}
    /** Clears GUI data (via OnClear()) before the base destructor frees
     * core model data panels may reference. */
    ~Controller() override { Clear(); }
  };

  GOResizable *p_MainWindow;
  GOConfig &r_config;
  GOMidiSystem &r_MidiSystem;

  GOMutex m_lock;
  bool m_OrganFileReady;
  GOOrganController *m_OrganController;

  GOMidiListener m_listener;

  /** The panel-creator objects that build this organ's built-in panels
   * (coupler, crescendo, divisionals, etc.). Temporary: the rest of the
   * GUI data these creators populate (m_OrganController's m_panels) will
   * move here too in a later step. */
  ptr_vector<GOGUIPanelCreator> m_PanelCreators;

  void OnMidiEvent(const GOMidiEvent &event) override;

  /** Builds and runs this organ's panel creators, called from
   * Controller::OnLoad() after LoadOrganGuiData() has built the base and
   * numbered panels the creators may add to.
   * @param cfg the config reader for the ODF/CMB currently being loaded */
  void LoadOrganGui(GOConfigReader &cfg);
  /** Undoes LoadOrganGui(). Called from Controller::OnClear() before
   * ClearOrganGuiData() frees the panels the creators may reference. */
  void ClearOrganGui();

  void SyncState();
  void CloseOrgan();

public:
  GOGuiOrgan(GOResizable *pMainWindow, GOConfig &config, GOMidiSystem &midi);
  ~GOGuiOrgan();

  GOOrganController *GetOrganController() const { return m_OrganController; }
  bool IsModified() const;

  /** @return the image cache for the currently loaded organ, still owned
   * by m_OrganController. */
  GOGuiImageCache &GetImageCache();
  /** @return the shared mouse state for the currently loaded organ's
   * panels, still owned by m_OrganController. */
  GOGUIMouseState &GetMouseState();
  /**
   * Registers a newly-built panel, called by a panel creator's
   * CreatePanels() while building this organ's GUI. Forwards to
   * m_OrganController, which still owns the panels array.
   * @param panel the panel to add; ownership transfers to m_OrganController
   */
  void AddPanel(GOGUIPanel *panel);

  void ShowPanel(unsigned id);
  void ShowOrganSettingsDialog();
  void ShowMidiList();
  void ShowStops();

  bool Save(const wxString &path = wxEmptyString);
  // Returns the loaded organ controller, or nullptr on failure.
  // Note: on failure CloseOrgan() is called, which also clears
  // m_OrganController.
  GOOrganController *LoadOrgan(
    const GOOrgan &organ,
    const wxString &cmb,
    bool isGuiOnly,
    GOProgressMonitor &monitor);
  bool UpdateCache(bool compress, GOProgressMonitor &monitor);

  void ShowMIDIEventDialog(
    GOMidiObject &obj, GOMidiDialogListener *pDialogListener = nullptr);
};

#endif